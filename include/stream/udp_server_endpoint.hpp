#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"
#include "stream/stream_stats.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

namespace stream
{

struct UdpServerConfig
{
    std::string dest_ip   = "127.0.0.1";
    uint16_t    dest_port = 9998;
    float       fec_ratio = 0.25f; // fraction of parity shards relative to data shards
};

// Sends H.264 frames over UDP with Reed-Solomon FEC sharding.
// Each frame is split into MTU-sized shards; ~25% extra parity shards are
// appended so the receiver can reconstruct the frame even with packet loss.
class UdpServerEndpoint
{
public:
    UdpServerEndpoint(
        UdpServerConfig              config,
        dc::ReceiverPort<Frame>&     frames,
        dc::SenderPort<ServerStats>& stats);
    ~UdpServerEndpoint();

    UdpServerEndpoint(const UdpServerEndpoint&)            = delete;
    UdpServerEndpoint& operator=(const UdpServerEndpoint&) = delete;

    void start();
    void stop();
    void notify_frame_available();

    uint64_t packets_sent() const { return m_packets_sent.load(); }

private:
    void run();
    void send_frame(class UdpSender& sender, const Frame& frame);
    void deliver_stats(const ServerStats& s);

    UdpServerConfig              m_config;
    dc::ReceiverPort<Frame>&     m_frames;
    dc::SenderPort<ServerStats>& m_stats;

    std::atomic<bool>       m_running{false};
    std::atomic<uint64_t>   m_packets_sent{0};
    std::thread             m_thread;
    std::mutex              m_frame_mutex;
    std::condition_variable m_frame_cv;

    uint16_t m_frame_index = 0;
    uint16_t m_seq_num     = 0;

    // Cached RS codec — rebuilt only when k or m changes (rare)
    mutable int                    m_codec_k = 0;
    mutable int                    m_codec_m = 0;
    mutable std::unique_ptr<class RsCodecWrapper> m_codec;
};

} // namespace stream
