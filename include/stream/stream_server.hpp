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

    ServerEndpointConfig         config_;
    dc::ReceiverPort<Frame>&     frames_;
    dc::SenderPort<ServerStats>& stats_;
    std::atomic<bool>            running_{false};
    std::thread                  thread_;
    std::mutex                   frame_mutex_;
    std::condition_variable      frame_cv_;
    std::mutex                   state_mutex_;
    class TcpListener*           listener_ = nullptr;
    class TcpConnection*         client_   = nullptr;
};

} // namespace stream
