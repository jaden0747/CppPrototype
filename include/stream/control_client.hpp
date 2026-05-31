#pragma once

#include "stream/control_protocol.hpp"

#include <cstdint>
#include <string>

namespace stream
{

// ---------------------------------------------------------------------------
// ControlClient — performs DESCRIBE/SETUP/PLAY/TEARDOWN handshake with server
//
// Synchronous API: each method blocks until the server responds.
// The caller is responsible for threading if needed.
// ---------------------------------------------------------------------------

struct ControlClientConfig
{
    std::string server_ip    = "127.0.0.1";
    uint16_t    control_port = 48010;
};

class ControlClient
{
public:
    ControlClient()  = default;
    ~ControlClient();

    ControlClient(ControlClient&& other) noexcept;
    ControlClient& operator=(ControlClient&& other) noexcept;

    ControlClient(const ControlClient&)            = delete;
    ControlClient& operator=(const ControlClient&) = delete;

    // Connect to control server. Returns true on success.
    bool connect(const ControlClientConfig& config);

    // DESCRIBE — get server capabilities
    bool describe(ServerCapabilities& out_caps);

    // SETUP — negotiate session parameters. Returns session token on success.
    bool setup(const SessionConfig& desired, std::string& out_token, SessionConfig& out_actual);

    // PLAY — start streaming for given session
    bool play(const std::string& session_token);

    // TEARDOWN — stop streaming
    bool teardown(const std::string& session_token);

    // Disconnect from control server
    void disconnect();

    bool is_connected() const;

private:
    bool send_request(const ControlRequest& req);
    bool recv_response(ControlResponse& resp);

    class TcpConnection* m_conn = nullptr;
};

} // namespace stream
