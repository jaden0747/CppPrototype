#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"
#include "stream/stream_stats.hpp"

#include <atomic>
#include <cstdint>
#include <thread>

namespace stream
{

struct UdpClientConfig
{
    uint16_t listen_port = 9998;
};

// Receives UDP shards, reassembles frames with Reed-Solomon FEC recovery,
// and delivers H.264 frames into the mempool for the GL decode pipeline.
class UdpClientEndpoint
{
public:
    UdpClientEndpoint(dc::SenderPort<Frame>& frames, dc::SenderPort<ClientStats>& stats);
    ~UdpClientEndpoint();

    UdpClientEndpoint(const UdpClientEndpoint&)            = delete;
    UdpClientEndpoint& operator=(const UdpClientEndpoint&) = delete;

    void start(UdpClientConfig config);
    void stop();
    bool is_running() const { return m_running.load(); }

private:
    void run();

    dc::SenderPort<Frame>&       m_frames;
    dc::SenderPort<ClientStats>& m_stats;

    std::atomic<bool> m_running{false};
    UdpClientConfig   m_config;
    std::thread       m_thread;
};

} // namespace stream
