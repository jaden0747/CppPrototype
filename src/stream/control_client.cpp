#include "stream/control_client.hpp"

#include "mylib/log.hpp"
#include "stream/tcp_socket.hpp"

#include <nlohmann/json.hpp>

#include <memory>

namespace stream
{

// ---------------------------------------------------------------------------
// Wire helpers (same format as control_server.cpp)
// ---------------------------------------------------------------------------

static bool write_json_message(TcpConnection& conn, const nlohmann::json& j)
{
    std::string payload = j.dump();
    uint32_t    len     = static_cast<uint32_t>(payload.size());
    uint8_t     header[4];
    header[0] = static_cast<uint8_t>((len >> 24) & 0xFF);
    header[1] = static_cast<uint8_t>((len >> 16) & 0xFF);
    header[2] = static_cast<uint8_t>((len >> 8) & 0xFF);
    header[3] = static_cast<uint8_t>(len & 0xFF);
    if (!conn.write_exact(header, 4))
        return false;
    return conn.write_exact(payload.data(), payload.size());
}

static bool read_json_message(TcpConnection& conn, nlohmann::json& out)
{
    uint8_t header[4];
    if (!conn.read_exact(header, 4))
        return false;
    uint32_t len = (static_cast<uint32_t>(header[0]) << 24) |
                   (static_cast<uint32_t>(header[1]) << 16) |
                   (static_cast<uint32_t>(header[2]) << 8) |
                   static_cast<uint32_t>(header[3]);
    if (len == 0 || len > 64 * 1024)
        return false;

    std::string buf(len, '\0');
    if (!conn.read_exact(buf.data(), len))
        return false;

    out = nlohmann::json::parse(buf, nullptr, false);
    return !out.is_discarded();
}

// ---------------------------------------------------------------------------
// ControlClient implementation
// ---------------------------------------------------------------------------

ControlClient::~ControlClient()
{
    disconnect();
}

ControlClient::ControlClient(ControlClient&& other) noexcept
    : m_conn(other.m_conn)
{
    other.m_conn = nullptr;
}

ControlClient& ControlClient::operator=(ControlClient&& other) noexcept
{
    if (this != &other)
    {
        disconnect();
        m_conn       = other.m_conn;
        other.m_conn = nullptr;
    }
    return *this;
}

bool ControlClient::connect(const ControlClientConfig& config)
{
    auto log = Log::get("ctrl");
    log->info("Connecting to control server {}:{}", config.server_ip, config.control_port);

    auto conn = connect_tcp(config.server_ip, config.control_port);
    if (!conn)
    {
        log->warn("Control connection refused");
        return false;
    }

    m_conn = new TcpConnection(std::move(*conn));
    log->info("Control channel established");
    return true;
}

void ControlClient::disconnect()
{
    if (m_conn)
    {
        m_conn->close();
        delete m_conn;
        m_conn = nullptr;
    }
}

bool ControlClient::is_connected() const
{
    return m_conn && m_conn->is_open();
}

bool ControlClient::send_request(const ControlRequest& req)
{
    if (!m_conn)
        return false;
    nlohmann::json j;
    to_json(j, req);
    return write_json_message(*m_conn, j);
}

bool ControlClient::recv_response(ControlResponse& resp)
{
    if (!m_conn)
        return false;
    nlohmann::json j;
    if (!read_json_message(*m_conn, j))
        return false;
    from_json(j, resp);
    return true;
}

bool ControlClient::describe(ServerCapabilities& out_caps)
{
    ControlRequest req;
    req.method = "DESCRIBE";

    if (!send_request(req))
        return false;

    ControlResponse resp;
    if (!recv_response(resp))
        return false;

    if (resp.status != 200)
        return false;

    from_json(resp.body, out_caps);
    return true;
}

bool ControlClient::setup(const SessionConfig& desired, std::string& out_token, SessionConfig& out_actual)
{
    ControlRequest req;
    req.method = "SETUP";
    to_json(req.body, desired);

    if (!send_request(req))
        return false;

    ControlResponse resp;
    if (!recv_response(resp))
        return false;

    if (resp.status != 200)
        return false;

    out_token = resp.body.value("session", "");
    if (out_token.empty())
        return false;

    if (resp.body.contains("config"))
        from_json(resp.body["config"], out_actual);
    else
        out_actual = desired;

    return true;
}

bool ControlClient::play(const std::string& session_token)
{
    ControlRequest req;
    req.method = "PLAY";
    req.body   = {{"session", session_token}};

    if (!send_request(req))
        return false;

    ControlResponse resp;
    if (!recv_response(resp))
        return false;

    return resp.status == 200;
}

bool ControlClient::teardown(const std::string& session_token)
{
    ControlRequest req;
    req.method = "TEARDOWN";
    req.body   = {{"session", session_token}};

    if (!send_request(req))
        return false;

    ControlResponse resp;
    if (!recv_response(resp))
        return false;

    return resp.status == 200;
}

} // namespace stream
