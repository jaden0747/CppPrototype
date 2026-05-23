#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"
#include "stream/stream_stats.hpp"

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

namespace stream
{

struct ClientEndpointConfig
{
    std::string server_ip   = "127.0.0.1";
    uint16_t    server_port = 9999;
};

class StreamClientEndpoint
{
public:
    StreamClientEndpoint(dc::SenderPort<Frame>& frames, dc::SenderPort<ClientStats>& stats);
    ~StreamClientEndpoint();

    StreamClientEndpoint(const StreamClientEndpoint&)            = delete;
    StreamClientEndpoint& operator=(const StreamClientEndpoint&) = delete;

    void start();
    void stop();
    void connect(ClientEndpointConfig config);
    void disconnect();

private:
    void run();
    void deliver_stats(const ClientStats& stats);
    void close_current_connection();

    dc::SenderPort<Frame>&       frames_;
    dc::SenderPort<ClientStats>& stats_;
    std::atomic<bool>            running_{false};
    std::atomic<bool>            connected_requested_{false};
    std::thread                  thread_;
    std::mutex                   request_mutex_;
    std::condition_variable      request_cv_;
    ClientEndpointConfig         config_;
    std::mutex                   state_mutex_;
    class TcpConnection*         connection_ = nullptr;
};

} // namespace stream
