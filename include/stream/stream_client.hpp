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

    dc::SenderPort<Frame>&       m_frames;
    dc::SenderPort<ClientStats>& m_stats;
    std::atomic<bool>            m_running{false};
    std::atomic<bool>            m_connected_requested{false};
    std::thread                  m_thread;
    std::mutex                   m_request_mutex;
    std::condition_variable      m_request_cv;
    ClientEndpointConfig         m_config;
    std::mutex                   m_state_mutex;
    class TcpConnection*         m_connection = nullptr;
};

} // namespace stream
