#include "stream/control_server.hpp"

#include "mylib/log.hpp"

#include <nlohmann/json.hpp>

#include <algorithm>
#include <random>

namespace stream
{

// ---------------------------------------------------------------------------
// Wire helpers: [4-byte big-endian length][JSON UTF-8 payload]
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
// ControlServer implementation
// ---------------------------------------------------------------------------

ControlServer::ControlServer(ControlServerConfig config)
    : m_config(std::move(config))
{
}

ControlServer::~ControlServer()
{
    stop();
}

void ControlServer::set_on_session_start(SessionStartCallback cb)
{
    m_on_start = std::move(cb);
}

void ControlServer::set_on_session_stop(SessionStopCallback cb)
{
    m_on_stop = std::move(cb);
}

void ControlServer::start()
{
    if (m_running.exchange(true))
        return;
    m_thread = std::thread(&ControlServer::run, this);
}

void ControlServer::stop()
{
    m_running.store(false);
    if (m_thread.joinable())
        m_thread.join();
}

std::string ControlServer::active_session() const
{
    std::lock_guard<std::mutex> lock(m_session_mutex);
    return m_active_session;
}

SessionConfig ControlServer::active_config() const
{
    std::lock_guard<std::mutex> lock(m_session_mutex);
    auto it = m_sessions.find(m_active_session);
    if (it != m_sessions.end())
        return it->second;
    return {};
}

bool ControlServer::has_active_session() const
{
    std::lock_guard<std::mutex> lock(m_session_mutex);
    return !m_active_session.empty();
}

std::string ControlServer::generate_token()
{
    static std::mt19937                    rng(std::random_device{}());
    static const char                      hex[] = "0123456789abcdef";
    std::uniform_int_distribution<int>     dist(0, 15);
    std::string                            token;
    token.reserve(16);
    for (int i = 0; i < 16; ++i)
        token += hex[dist(rng)];
    return token;
}

bool ControlServer::send_response(TcpConnection& conn, const ControlResponse& resp)
{
    nlohmann::json j;
    to_json(j, resp);
    return write_json_message(conn, j);
}

bool ControlServer::recv_request(TcpConnection& conn, ControlRequest& req)
{
    nlohmann::json j;
    if (!read_json_message(conn, j))
        return false;
    from_json(j, req);
    return true;
}

void ControlServer::run()
{
    auto log = Log::get("ctrl");
    log->info("Control server listening on TCP :{}", m_config.control_port);

    TcpListener listener(m_config.control_port);
    if (!listener.is_open())
    {
        log->error("Failed to bind control port {}", m_config.control_port);
        return;
    }

    while (m_running)
    {
        auto accepted = listener.accept_for(m_config.accept_timeout);
        if (!m_running)
            break;
        if (!accepted)
            continue;

        // Single-client: handle one session at a time (mirrors Sunshine behavior)
        handle_client(std::move(*accepted));
    }

    log->info("Control server stopped");
}

void ControlServer::handle_client(TcpConnection conn)
{
    auto        log           = Log::get("ctrl");
    std::string session_token;

    log->info("Control client connected");

    while (m_running && conn.is_open())
    {
        ControlRequest req;
        if (!recv_request(conn, req))
            break;

        log->info("CTRL << {} {}", req.method, req.body.dump());
        ControlResponse resp;

        if (req.method == "DESCRIBE")
        {
            ServerCapabilities caps;
            caps.video_port = m_config.video_port;
            nlohmann::json body;
            to_json(body, caps);
            resp.status = 200;
            resp.body   = body;
        }
        else if (req.method == "SETUP")
        {
            SessionConfig config;
            if (req.body.contains("width"))
                config.width = req.body["width"].get<int>();
            if (req.body.contains("height"))
                config.height = req.body["height"].get<int>();
            if (req.body.contains("fps"))
                config.fps = req.body["fps"].get<int>();
            if (req.body.contains("bitrate"))
                config.bitrate = req.body["bitrate"].get<int>();
            if (req.body.contains("codec"))
                config.codec = req.body["codec"].get<std::string>();
            if (req.body.contains("fec_ratio"))
                config.fec_ratio = req.body["fec_ratio"].get<float>();

            session_token = generate_token();
            {
                std::lock_guard<std::mutex> lock(m_session_mutex);
                m_sessions[session_token] = config;
            }

            resp.status = 200;
            resp.body   = {{"session", session_token}};
            nlohmann::json cfg_json;
            to_json(cfg_json, config);
            resp.body["config"] = cfg_json;

            log->info("Session created: {} ({}x{} @{}fps, {}bps)",
                      session_token, config.width, config.height, config.fps, config.bitrate);
        }
        else if (req.method == "PLAY")
        {
            std::string token = req.body.value("session", session_token);
            std::lock_guard<std::mutex> lock(m_session_mutex);
            auto it = m_sessions.find(token);
            if (it == m_sessions.end())
            {
                resp.status = 404;
                resp.body   = {{"error", "Session not found"}};
            }
            else
            {
                m_active_session = token;
                resp.status      = 200;
                resp.body        = {{"message", "Streaming started"}};
                log->info("PLAY session {}", token);

                if (m_on_start)
                    m_on_start(token, it->second);
            }
        }
        else if (req.method == "TEARDOWN")
        {
            std::string token = req.body.value("session", session_token);
            {
                std::lock_guard<std::mutex> lock(m_session_mutex);
                m_sessions.erase(token);
                if (m_active_session == token)
                    m_active_session.clear();
            }
            resp.status = 200;
            resp.body   = {{"message", "Session torn down"}};
            log->info("TEARDOWN session {}", token);

            if (m_on_stop)
                m_on_stop(token);

            send_response(conn, resp);
            break; // Client done
        }
        else
        {
            resp.status = 400;
            resp.body   = {{"error", "Unknown method: " + req.method}};
        }

        if (!send_response(conn, resp))
            break;
    }

    // Clean up if client disconnected without TEARDOWN
    if (!session_token.empty())
    {
        std::lock_guard<std::mutex> lock(m_session_mutex);
        if (m_sessions.count(session_token))
        {
            m_sessions.erase(session_token);
            if (m_active_session == session_token)
            {
                m_active_session.clear();
                if (m_on_stop)
                    m_on_stop(session_token);
            }
        }
    }

    log->info("Control client disconnected");
}

} // namespace stream
