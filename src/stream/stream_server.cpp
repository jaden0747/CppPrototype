#include "stream/stream_server.hpp"

#include "mylib/log.hpp"
#include "stream/frame_io.hpp"
#include "stream/tcp_socket.hpp"

#include <chrono>

namespace stream
{

StreamServerEndpoint::StreamServerEndpoint(
    ServerEndpointConfig         config,
    dc::ReceiverPort<Frame>&     frames,
    dc::SenderPort<ServerStats>& stats)
    : m_config(config)
    , m_frames(frames)
    , m_stats(stats)
{
}

StreamServerEndpoint::~StreamServerEndpoint()
{
    stop();
}

void StreamServerEndpoint::start()
{
    if (m_running.exchange(true))
        return;
    m_thread = std::thread(&StreamServerEndpoint::run, this);
}

void StreamServerEndpoint::stop()
{
    if (!m_running.exchange(false))
        return;

    {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        if (m_client)
            m_client->close();
        if (m_listener)
            m_listener->close();
    }
    m_frame_cv.notify_all();

    if (m_thread.joinable())
        m_thread.join();
}

void StreamServerEndpoint::notify_frame_available()
{
    m_frame_cv.notify_one();
}

void StreamServerEndpoint::deliver_stats(const ServerStats& stats)
{
    ServerStats* slot = m_stats.reserve();
    if (!slot)
        return;
    *slot = stats;
    m_stats.deliver();
}

void StreamServerEndpoint::run()
{
    auto        log = Log::get("net");
    TcpListener listener(m_config.listen_port);
    {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        m_listener = &listener;
    }

    if (!listener.is_open())
    {
        log->error("Failed to listen on TCP :{}", m_config.listen_port);
        return;
    }

    log->info("Listening on TCP :{}", m_config.listen_port);

    while (m_running)
    {
        auto accepted = listener.accept_for(m_config.accept_wait);
        if (!m_running)
            break;
        if (!accepted)
            continue;

        TcpConnection client = std::move(*accepted);
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            m_client = &client;
        }

        ServerStats snapshot;
        snapshot.connected = true;
        snapshot.peer      = "client";
        log->info("Client connected");
        deliver_stats(snapshot);

        while (m_running && client.is_open())
        {
            {
                std::unique_lock<std::mutex> lock(m_frame_mutex);
                m_frame_cv.wait_for(lock, m_config.frame_wait);
            }
            if (!m_running)
                break;

            m_frames.update();
            if (!m_frames.hasNewData())
            {
                m_frames.cleanup();
                continue;
            }

            const Frame* frame = m_frames.getData();
            if (!frame || !frame->valid())
            {
                m_frames.cleanup();
                continue;
            }

            if (!write_frame(client, *frame))
            {
                log->warn("write_frame failed; client disconnected");
                m_frames.cleanup();
                break;
            }

            snapshot.bytes_sent += frame_header_wire_size + frame->pixels.size();
            snapshot.frames_sent++;
            deliver_stats(snapshot);
            m_frames.cleanup();

            if (snapshot.frames_sent % 300 == 0)
                log->debug(
                    "Sent {} frames ({:.2f} MB)", snapshot.frames_sent, static_cast<double>(snapshot.bytes_sent) / 1e6);
        }

        client.close();
        {
            std::lock_guard<std::mutex> lock(m_state_mutex);
            if (m_client == &client)
                m_client = nullptr;
        }

        log->info("Client disconnected");
        deliver_stats(ServerStats{});
    }

    {
        std::lock_guard<std::mutex> lock(m_state_mutex);
        m_listener = nullptr;
    }
    log->info("Net thread exited");
}

} // namespace stream
