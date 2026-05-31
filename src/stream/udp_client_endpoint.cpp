#include "stream/udp_client_endpoint.hpp"

#include "mylib/log.hpp"
#include "stream/rs_fec.hpp"
#include "stream/udp_packet.hpp"
#include "stream/udp_socket.hpp"

#include <cstring>
#include <unordered_map>
#include <vector>

namespace stream
{

// ---------------------------------------------------------------------------
// FrameBuffer — storage for shards belonging to one frame index
// ---------------------------------------------------------------------------

struct FrameBuffer
{
    int                               total_shards = 0;
    int                               fec_shards   = 0;
    std::vector<std::vector<uint8_t>> shards;  // [total_shards][UDP_PAYLOAD_SIZE]
    std::vector<uint8_t>              present;
    int                               count     = 0;
    bool                              assembled = false;
};

// ---------------------------------------------------------------------------
// Reassembler — collects shards and reconstructs frames with FEC recovery.
//
// Uses 16-bit unsigned sequence arithmetic for wrap-around safety:
//   "fi is newer than newest" ⟺  (uint16_t)(fi − newest) < 0x8000
//   "fi is old (>WINDOW behind newest)" ⟺ diff > WINDOW && diff < 0x8000
// ---------------------------------------------------------------------------

class Reassembler
{
    static constexpr uint16_t WINDOW = 60; // frames to keep alive before discarding

public:
    std::vector<uint8_t> process(const ShardHeader& hdr, const uint8_t* payload)
    {
        if (hdr.total_shards == 0 || hdr.fec_shards >= hdr.total_shards)
            return {};

        const uint16_t fi = hdr.frame_index;

        if (is_old(fi))
            return {};

        // Advance m_newest when fi is strictly ahead in circular sense
        if (m_buffers.empty() || static_cast<uint16_t>(fi - m_newest) < 0x8000u)
            m_newest = fi;

        auto& buf = m_buffers[fi];
        if (buf.assembled)
            return {};

        // Allocate shard storage on first shard for this frame
        if (buf.total_shards == 0)
        {
            buf.total_shards = hdr.total_shards;
            buf.fec_shards   = hdr.fec_shards;
            buf.shards.assign(static_cast<size_t>(hdr.total_shards),
                              std::vector<uint8_t>(UDP_PAYLOAD_SIZE, 0));
            buf.present.assign(static_cast<size_t>(hdr.total_shards), false);
        }

        const uint8_t si = hdr.shard_index;
        if (si >= buf.total_shards || buf.present[si])
            return {};

        std::memcpy(buf.shards[static_cast<size_t>(si)].data(), payload, UDP_PAYLOAD_SIZE);
        buf.present[static_cast<size_t>(si)] = true;
        buf.count++;

        const int k = buf.total_shards - buf.fec_shards;
        if (buf.count < k)
            return {};

        return assemble(fi, buf);
    }

    uint64_t fec_recoveries() const { return m_fec_recoveries; }

private:
    bool is_old(uint16_t fi) const
    {
        if (m_buffers.empty())
            return false;
        const uint16_t diff = static_cast<uint16_t>(m_newest - fi);
        // diff ≥ 0x8000 means fi is ahead of m_newest (future frame) — not old
        return diff > WINDOW && diff < 0x8000u;
    }

    std::vector<uint8_t> assemble(uint16_t fi, FrameBuffer& buf)
    {
        const int k = buf.total_shards - buf.fec_shards;

        // Check if FEC recovery is needed
        bool need_fec = false;
        for (int i = 0; i < k; ++i)
            if (!buf.present[static_cast<size_t>(i)])
            {
                need_fec = true;
                break;
            }

        if (need_fec)
        {
            std::vector<uint8_t*> ptrs(static_cast<size_t>(buf.total_shards));
            for (int i = 0; i < buf.total_shards; ++i)
                ptrs[static_cast<size_t>(i)] = buf.shards[static_cast<size_t>(i)].data();

            rs::Codec codec(k, buf.fec_shards);
            if (!codec.decode(ptrs.data(), buf.present.data(), UDP_PAYLOAD_SIZE))
                return {}; // not enough shards yet

            m_fec_recoveries++;
        }

        // Concatenate the k data shards
        std::vector<uint8_t> frame_data(static_cast<size_t>(k) * UDP_PAYLOAD_SIZE);
        for (int i = 0; i < k; ++i)
            std::memcpy(
                frame_data.data() + static_cast<size_t>(i) * UDP_PAYLOAD_SIZE,
                buf.shards[static_cast<size_t>(i)].data(),
                UDP_PAYLOAD_SIZE);

        buf.assembled = true;
        prune();
        return frame_data;
    }

    void prune()
    {
        for (auto it = m_buffers.begin(); it != m_buffers.end();)
        {
            if (it->second.assembled || is_old(it->first))
                it = m_buffers.erase(it);
            else
                ++it;
        }
    }

    std::unordered_map<uint16_t, FrameBuffer> m_buffers;
    uint16_t                                  m_newest         = 0;
    uint64_t                                  m_fec_recoveries = 0;
};

// ---------------------------------------------------------------------------
// UdpClientEndpoint
// ---------------------------------------------------------------------------

UdpClientEndpoint::UdpClientEndpoint(
    dc::SenderPort<Frame>&       frames,
    dc::SenderPort<ClientStats>& stats)
    : m_frames(frames)
    , m_stats(stats)
{
}

UdpClientEndpoint::~UdpClientEndpoint()
{
    stop();
}

void UdpClientEndpoint::start(UdpClientConfig config)
{
    if (m_running.exchange(true))
        return;
    m_config = config;
    m_thread = std::thread(&UdpClientEndpoint::run, this);
}

void UdpClientEndpoint::stop()
{
    if (!m_running.exchange(false))
        return;
    if (m_thread.joinable())
        m_thread.join();
}

void UdpClientEndpoint::run()
{
    auto log = Log::get("net");

    UdpReceiver receiver;
    if (!receiver.bind(m_config.listen_port))
    {
        log->error("UDP receiver: bind failed on port {}", m_config.listen_port);
        ClientStats err;
        err.connected = false;
        err.status    = "Bind failed on :" + std::to_string(m_config.listen_port);
        if (ClientStats* s = m_stats.reserve()) { *s = err; m_stats.deliver(); }
        return;
    }
    log->info("UDP client listening on :{}", m_config.listen_port);

    ClientStats snapshot;
    snapshot.connected = true;
    snapshot.status    = "Listening on :" + std::to_string(m_config.listen_port);
    if (ClientStats* s = m_stats.reserve()) { *s = snapshot; m_stats.deliver(); }

    Reassembler          reassembler;
    FpsCounter           fps(1.0f);
    std::vector<uint8_t> recv_buf(UDP_MTU);

    while (m_running)
    {
        const int n = receiver.receive(recv_buf.data(), recv_buf.size());

        if (n < 0)
            break; // socket closed / error

        if (n == 0)
            continue; // timeout — loop and check m_running

        if (n < static_cast<int>(UDP_HEADER_SIZE))
            continue; // malformed packet

        const ShardHeader hdr     = decode_shard_header(recv_buf.data());
        const uint8_t*    payload = recv_buf.data() + UDP_HEADER_SIZE;

        if (hdr.shard_index >= hdr.total_shards || hdr.total_shards == 0)
            continue;

        snapshot.bytes_received += static_cast<uint64_t>(n);

        auto frame_data = reassembler.process(hdr, payload);
        if (frame_data.empty())
            continue;

        // Deliver assembled H.264 NAL bytes to the mempool for decoding
        Frame* slot = m_frames.reserve();
        if (slot)
        {
            slot->pixels      = std::move(frame_data);
            slot->width       = 0;
            slot->height      = 0;
            slot->format      = PixelFormat::Rgb8;
            slot->compression = Compression::H264;
            m_frames.deliver();
        }

        snapshot.frames_received++;
        snapshot.fps       = fps.tick();
        snapshot.connected = true;
        snapshot.status    = "Receiving";
        if (ClientStats* s = m_stats.reserve()) { *s = snapshot; m_stats.deliver(); }

        if (snapshot.frames_received % 300 == 0)
            log->debug("Assembled {} frames ({:.2f} MB, {} FEC recoveries)",
                       snapshot.frames_received,
                       static_cast<double>(snapshot.bytes_received) / 1e6,
                       reassembler.fec_recoveries());
    }

    receiver.close();
    snapshot.connected = false;
    snapshot.status    = "Stopped";
    if (ClientStats* s = m_stats.reserve()) { *s = snapshot; m_stats.deliver(); }
    log->info("UDP client thread exited");
}

} // namespace stream
