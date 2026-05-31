#pragma once

#include "stream/control_protocol.hpp"
#include "stream/tcp_socket.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>

namespace stream
{

// ---------------------------------------------------------------------------
// ControlServer — TCP control channel that handles RTSP-like session negotiation
//
// Flow:
//   1. Client connects to control port
//   2. Client sends DESCRIBE -> server replies with capabilities
//   3. Client sends SETUP {config} -> server creates session, replies with token
//   4. Client sends PLAY {session} -> server starts streaming, replies OK
//   5. Client sends TEARDOWN {session} -> server stops streaming
//
// The server notifies the application layer via callbacks when a session
// transitions to PLAY or TEARDOWN state.
// ---------------------------------------------------------------------------

struct ControlServerConfig
{
    uint16_t                  control_port = 48010;
    uint16_t                  video_port   = 9998;
    std::chrono::milliseconds accept_timeout{1000};
};

// Callback fired when a client session starts or stops streaming.
using SessionStartCallback = std::function<void(const std::string& session_token, const SessionConfig& config)>;
using SessionStopCallback  = std::function<void(const std::string& session_token)>;

class ControlServer
{
public:
    explicit ControlServer(ControlServerConfig config);
    ~ControlServer();

    ControlServer(const ControlServer&)            = delete;
    ControlServer& operator=(const ControlServer&) = delete;

    void set_on_session_start(SessionStartCallback cb);
    void set_on_session_stop(SessionStopCallback cb);

    void start();
    void stop();

    // Query active session (empty string if none)
    std::string      active_session() const;
    SessionConfig    active_config() const;
    bool             has_active_session() const;

private:
    void run();
    void handle_client(TcpConnection conn);

    bool send_response(TcpConnection& conn, const ControlResponse& resp);
    bool recv_request(TcpConnection& conn, ControlRequest& req);

    std::string generate_token();

    ControlServerConfig m_config;

    std::atomic<bool> m_running{false};
    std::thread       m_thread;

    mutable std::mutex                          m_session_mutex;
    std::unordered_map<std::string, SessionConfig> m_sessions;
    std::string                                 m_active_session;

    SessionStartCallback m_on_start;
    SessionStopCallback  m_on_stop;
};

} // namespace stream
