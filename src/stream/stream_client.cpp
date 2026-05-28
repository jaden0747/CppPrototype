#include "stream/stream_client.hpp"

#include "mylib/log.hpp"
#include "stream/frame_io.hpp"
#include "stream/tcp_socket.hpp"

#include <chrono>
#include <utility>

namespace stream
{

StreamClientEndpoint::StreamClientEndpoint(dc::SenderPort<Frame>& frames, dc::SenderPort<ClientStats>& stats)
    : m_frames(frames)
    , m_stats(stats)
{
}

StreamClientEndpoint::~StreamClientEndpoint()
{
    stop();
}

void StreamClientEndpoint::start()
{
    if (m_running.exchange(true))
        return;
    m_thread = std::thread(&StreamClientEndpoint::run, this);
}

void StreamClientEndpoint::stop()
{
    if (!m_running.exchange(false))
        return;

    m_connected_requested.store(false);
    close_current_connection();
    m_request_cv.notify_all();

    if (m_thread.joinable())
        m_thread.join();
}

void StreamClientEndpoint::connect(ClientEndpointConfig config)
{
    {
        std::lock_guard<std::mutex> lock(m_request_mutex);
        m_config = std::move(config);
        m_connected_requested.store(true);
    }
    m_request_cv.notify_all();
}

void StreamClientEndpoint::disconnect()
{
    m_connected_requested.store(false);
    close_current_connection();
    m_request_cv.notify_all();
}

void StreamClientEndpoint::deliver_stats(const ClientStats& stats)
{
    ClientStats* slot = m_stats.reserve();
    if (!slot)
        return;
    *slot = stats;
    m_stats.deliver();
}

void StreamClientEndpoint::close_current_connection()
{
    std::lock_guard<std::mutex> lock(m_state_mutex);
    if (m_connection)
        m_connection->close();
}

void StreamClientEndpoint::run()
{
    auto log = Log::get("net");

    while (m_running)
    {
        ClientEndpointConfig config;
        {
            std::unique_lock<std::mutex> lock(m_request_mutex);
            m_request_cv.wait(lock, [&] { return !m_running || m_connected_requested.load(); });
            if (!m_running)
                break;
            config = m_config;
        }

        log->info("Connecting to {}:{}...", config.server_ip, config.server_port);
        deliver_stats(
            ClientStats{
                0, 0, 0.0f, false, "Connecting to " + config.server_ip + ":" + std::to_string(config.server_port)});

        auto maybe_conn = connect_tcp(config.server_ip, config.server_port);
        if (!maybe_conn)
        {
            log->warn("Connection refused to {}:{}", config.server_ip, config.server_port);
            deliver_stats(ClientStats{0, 0, 0.0f, false, "Connection refused"});
            m_connected_requested.store(false);
            continue;
        }

        TcpConnection conn = std::move(*maybe_conn);
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            m_connection = &conn;
        }

        log->info("Connected to {}:{}", config.server_ip, config.server_port);
        ClientStats snapshot;
        snapshot.connected = true;
        snapshot.status    = "Connected";
        deliver_stats(snapshot);

        FpsCounter fps(1.0f);
        while (m_running && m_connected_requested.load() && conn.is_open())
        {
            auto header = read_frame_header(conn);
            if (!header)
            {
                log->warn("recv header failed; server disconnected");
                break;
            }

            Frame* slot = m_frames.reserve();
            if (slot)
            {
                if (!read_frame_payload(conn, *header, *slot))
                {
                    log->warn("recv pixels failed; server disconnected");
                    break;
                }
                m_frames.deliver();
            }
            else
            {
                log->warn("Frame pool exhausted; draining {} bytes", header->payload_size);
                if (!drain_frame_payload(conn, *header))
                    break;
            }

            snapshot.bytes_received += frame_header_wire_size + header->payload_size;
            snapshot.frames_received++;
            snapshot.fps       = fps.tick();
            snapshot.connected = true;
            snapshot.status    = "Connected";
            deliver_stats(snapshot);

            if (snapshot.frames_received % 300 == 0)
                log->debug(
                    "Received {} frames ({:.2f} MB)",
                    snapshot.frames_received,
                    static_cast<double>(snapshot.bytes_received) / 1e6);
        }

        conn.close();
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_connection == &conn)
                m_connection = nullptr;
        }

        log->info("Disconnected from {}:{}", config.server_ip, config.server_port);
        m_connected_requested.store(false);
        deliver_stats(ClientStats{0, 0, 0.0f, false, "Disconnected"});
    }

    log->info("Net thread exited");
}

} // namespace stream
