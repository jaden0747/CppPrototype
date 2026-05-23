#include "stream/stream_client.hpp"

#include "mylib/log.hpp"
#include "stream/frame_io.hpp"
#include "stream/tcp_socket.hpp"

#include <chrono>
#include <utility>

namespace stream
{

StreamClientEndpoint::StreamClientEndpoint(dc::SenderPort<Frame>& frames, dc::SenderPort<ClientStats>& stats)
    : frames_(frames)
    , stats_(stats)
{
}

StreamClientEndpoint::~StreamClientEndpoint()
{
    stop();
}

void StreamClientEndpoint::start()
{
    if (running_.exchange(true))
        return;
    thread_ = std::thread(&StreamClientEndpoint::run, this);
}

void StreamClientEndpoint::stop()
{
    if (!running_.exchange(false))
        return;

    connected_requested_.store(false);
    close_current_connection();
    request_cv_.notify_all();

    if (thread_.joinable())
        thread_.join();
}

void StreamClientEndpoint::connect(ClientEndpointConfig config)
{
    {
        std::lock_guard<std::mutex> lock(request_mutex_);
        config_ = std::move(config);
        connected_requested_.store(true);
    }
    request_cv_.notify_all();
}

void StreamClientEndpoint::disconnect()
{
    connected_requested_.store(false);
    close_current_connection();
    request_cv_.notify_all();
}

void StreamClientEndpoint::deliver_stats(const ClientStats& stats)
{
    ClientStats* slot = stats_.reserve();
    if (!slot)
        return;
    *slot = stats;
    stats_.deliver();
}

void StreamClientEndpoint::close_current_connection()
{
    std::lock_guard<std::mutex> lock(state_mutex_);
    if (connection_)
        connection_->close();
}

void StreamClientEndpoint::run()
{
    auto log = Log::get("net");

    while (running_)
    {
        ClientEndpointConfig config;
        {
            std::unique_lock<std::mutex> lock(request_mutex_);
            request_cv_.wait(lock, [&] { return !running_ || connected_requested_.load(); });
            if (!running_)
                break;
            config = config_;
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
            connected_requested_.store(false);
            continue;
        }

        TcpConnection conn = std::move(*maybe_conn);
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            connection_ = &conn;
        }

        log->info("Connected to {}:{}", config.server_ip, config.server_port);
        ClientStats snapshot;
        snapshot.connected = true;
        snapshot.status    = "Connected";
        deliver_stats(snapshot);

        FpsCounter fps(0.1f);
        while (running_ && connected_requested_.load() && conn.is_open())
        {
            auto header = read_frame_header(conn);
            if (!header)
            {
                log->warn("recv header failed; server disconnected");
                break;
            }

            Frame* slot = frames_.reserve();
            if (slot)
            {
                if (!read_frame_payload(conn, *header, *slot))
                {
                    log->warn("recv pixels failed; server disconnected");
                    break;
                }
                frames_.deliver();
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
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (connection_ == &conn)
                connection_ = nullptr;
        }

        log->info("Disconnected from {}:{}", config.server_ip, config.server_port);
        connected_requested_.store(false);
        deliver_stats(ClientStats{0, 0, 0.0f, false, "Disconnected"});
    }

    log->info("Net thread exited");
}

} // namespace stream
