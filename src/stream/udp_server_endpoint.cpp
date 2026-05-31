#include "stream/udp_server_endpoint.hpp"

#include "mylib/log.hpp"
#include "stream/rs_fec.hpp"
#include "stream/udp_packet.hpp"
#include "stream/udp_socket.hpp"

#include <algorithm>
#include <array>
#include <cstring>

namespace stream
{

// Thin wrapper so rs::Codec doesn't appear in the public header
class RsCodecWrapper
{
public:
    RsCodecWrapper(int k, int m) : m_codec(k, m) {}
    const rs::Codec& codec() const { return m_codec; }

private:
    rs::Codec m_codec;
};

UdpServerEndpoint::UdpServerEndpoint(
    UdpServerConfig              config,
    dc::ReceiverPort<Frame>&     frames,
    dc::SenderPort<ServerStats>& stats)
    : m_config(std::move(config))
    , m_frames(frames)
    , m_stats(stats)
{
}

UdpServerEndpoint::~UdpServerEndpoint()
{
    stop();
}

void UdpServerEndpoint::start()
{
    if (m_running.exchange(true))
        return;
    m_thread = std::thread(&UdpServerEndpoint::run, this);
}

void UdpServerEndpoint::stop()
{
    if (!m_running.exchange(false))
        return;
    m_frame_cv.notify_all();
    if (m_thread.joinable())
        m_thread.join();
}

void UdpServerEndpoint::notify_frame_available()
{
    m_frame_cv.notify_one();
}

void UdpServerEndpoint::deliver_stats(const ServerStats& s)
{
    ServerStats* slot = m_stats.reserve();
    if (!slot)
        return;
    *slot = s;
    m_stats.deliver();
}

// ── send_frame ────────────────────────────────────────────────────────────
// Splits frame pixels into MTU-sized data shards, appends FEC parity shards,
// and fires them all via UDP.  All shards are exactly UDP_PAYLOAD_SIZE bytes
// (last data shard is zero-padded); parity is computed over GF(2^8).

void UdpServerEndpoint::send_frame(UdpSender& sender, const Frame& frame)
{
    const uint8_t* data       = frame.pixels.data();
    const size_t   data_size  = frame.pixels.size();
    const int      chunk_size = static_cast<int>(UDP_PAYLOAD_SIZE);

    // Compute shard counts
    const int k     = static_cast<int>((data_size + static_cast<size_t>(chunk_size) - 1) / chunk_size);
    const int m     = std::max(1, static_cast<int>(static_cast<float>(k) * m_config.fec_ratio + 0.5f));
    const int total = k + m;

    if (total > 255)
    {
        Log::get("net")->warn("Frame too large for UDP sharding ({} bytes, {} shards), skipping", data_size, total);
        return;
    }

    // Build data shards — zero-pad the last one to chunk_size
    std::vector<std::vector<uint8_t>> storage(
        static_cast<size_t>(total),
        std::vector<uint8_t>(static_cast<size_t>(chunk_size), 0));

    for (int i = 0; i < k; ++i)
    {
        const size_t off = static_cast<size_t>(i * chunk_size);
        const size_t len = std::min(static_cast<size_t>(chunk_size), data_size - off);
        std::memcpy(storage[static_cast<size_t>(i)].data(), data + off, len);
    }

    // Build pointer table for RS codec
    std::vector<uint8_t*> ptrs(static_cast<size_t>(total));
    for (int i = 0; i < total; ++i)
        ptrs[static_cast<size_t>(i)] = storage[static_cast<size_t>(i)].data();

    // Rebuild codec only when shard layout changes (typically once per stream)
    if (!m_codec || m_codec_k != k || m_codec_m != m)
    {
        m_codec   = std::make_unique<RsCodecWrapper>(k, m);
        m_codec_k = k;
        m_codec_m = m;
    }
    m_codec->codec().encode(ptrs.data(), chunk_size);

    // Send all total shards
    std::array<uint8_t, UDP_MTU> pkt{};

    for (int i = 0; i < total; ++i)
    {
        ShardHeader hdr;
        hdr.seq_num      = m_seq_num++;
        hdr.frame_index  = m_frame_index;
        hdr.shard_index  = static_cast<uint8_t>(i);
        hdr.total_shards = static_cast<uint8_t>(total);
        hdr.fec_shards   = static_cast<uint8_t>(m);
        hdr.flags        = static_cast<uint8_t>(
            (i == 0         ? SHARD_FLAG_FIRST : 0u) |
            (i == total - 1 ? SHARD_FLAG_LAST  : 0u));

        encode_shard_header(pkt.data(), hdr);
        std::memcpy(pkt.data() + UDP_HEADER_SIZE,
                    storage[static_cast<size_t>(i)].data(),
                    static_cast<size_t>(chunk_size));

        sender.send(pkt.data(), UDP_HEADER_SIZE + static_cast<size_t>(chunk_size));
        m_packets_sent.fetch_add(1, std::memory_order_relaxed);
    }

    m_frame_index++;
}

void UdpServerEndpoint::run()
{
    auto log = Log::get("net");

    UdpSender sender;
    if (!sender.open(m_config.dest_ip, m_config.dest_port))
    {
        log->error("UDP sender failed to open (dest={}:{})", m_config.dest_ip, m_config.dest_port);
        return;
    }
    log->info("UDP server → {}:{} (fec={:.0f}%)",
              m_config.dest_ip, m_config.dest_port,
              m_config.fec_ratio * 100.0f);

    ServerStats snapshot;
    snapshot.connected = true;
    snapshot.peer      = m_config.dest_ip + ":" + std::to_string(m_config.dest_port);
    deliver_stats(snapshot);

    while (m_running)
    {
        {
            std::unique_lock<std::mutex> lk(m_frame_mutex);
            m_frame_cv.wait_for(lk, std::chrono::milliseconds(200));
        }
        if (!m_running)
            break;

        m_frames.update();
        if (!m_frames.hasNewData())
        {
            m_frames.cleanup();
            continue;
        }

        const Frame* frame = m_frames.getData();
        if (!frame || frame->pixels.empty())
        {
            m_frames.cleanup();
            continue;
        }

        send_frame(sender, *frame);

        snapshot.frames_sent++;
        snapshot.bytes_sent += frame->pixels.size();
        deliver_stats(snapshot);
        m_frames.cleanup();

        if (snapshot.frames_sent % 300 == 0)
            log->debug("Sent {} frames ({:.2f} MB, {} pkts)",
                       snapshot.frames_sent,
                       static_cast<double>(snapshot.bytes_sent) / 1e6,
                       m_packets_sent.load());
    }

    sender.close();
    deliver_stats(ServerStats{});
    log->info("UDP server thread exited");
}

} // namespace stream
