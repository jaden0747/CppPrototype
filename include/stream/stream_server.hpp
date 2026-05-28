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
#include <thread>

namespace stream
{

struct ServerEndpointConfig
{
    uint16_t                  listen_port = 9999;
    std::chrono::milliseconds accept_wait{1000};
    std::chrono::milliseconds frame_wait{200};
};

class StreamServerEndpoint
{
public:
    StreamServerEndpoint(
        ServerEndpointConfig         config,
        dc::ReceiverPort<Frame>&     frames,
        dc::SenderPort<ServerStats>& stats);
    ~StreamServerEndpoint();

    StreamServerEndpoint(const StreamServerEndpoint&)            = delete;
    StreamServerEndpoint& operator=(const StreamServerEndpoint&) = delete;

    void start();
    void stop();
    void notify_frame_available();

private:
    void run();
    void deliver_stats(const ServerStats& stats);

    ServerEndpointConfig         m_config;
    dc::ReceiverPort<Frame>&     m_frames;
    dc::SenderPort<ServerStats>& m_stats;
    std::atomic<bool>            m_running{false};
    std::thread                  m_thread;
    std::mutex                   m_frame_mutex;
    std::condition_variable      m_frame_cv;
    std::mutex                   m_state_mutex;
    class TcpListener*           m_listener = nullptr;
    class TcpConnection*         m_client   = nullptr;
};

} // namespace stream
