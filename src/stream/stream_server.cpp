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
    : config_(config)
    , frames_(frames)
    , stats_(stats)
{
}

StreamServerEndpoint::~StreamServerEndpoint()
{
    stop();
}

void StreamServerEndpoint::start()
{
    if (running_.exchange(true))
        return;
    thread_ = std::thread(&StreamServerEndpoint::run, this);
}

void StreamServerEndpoint::stop()
{
    if (!running_.exchange(false))
        return;

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        if (client_)
            client_->close();
        if (listener_)
            listener_->close();
    }
    frame_cv_.notify_all();

    if (thread_.joinable())
        thread_.join();
}

void StreamServerEndpoint::notify_frame_available()
{
    frame_cv_.notify_one();
}

void StreamServerEndpoint::deliver_stats(const ServerStats& stats)
{
    ServerStats* slot = stats_.reserve();
    if (!slot)
        return;
    *slot = stats;
    stats_.deliver();
}

void StreamServerEndpoint::run()
{
    auto        log = Log::get("net");
    TcpListener listener(config_.listen_port);
    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        listener_ = &listener;
    }

    if (!listener.is_open())
    {
        log->error("Failed to listen on TCP :{}", config_.listen_port);
        return;
    }

    log->info("Listening on TCP :{}", config_.listen_port);

    while (running_)
    {
        auto accepted = listener.accept_for(config_.accept_wait);
        if (!running_)
            break;
        if (!accepted)
            continue;

        TcpConnection client = std::move(*accepted);
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            client_ = &client;
        }

        ServerStats snapshot;
        snapshot.connected = true;
        snapshot.peer      = "client";
        log->info("Client connected");
        deliver_stats(snapshot);

        while (running_ && client.is_open())
        {
            {
                std::unique_lock<std::mutex> lock(frame_mutex_);
                frame_cv_.wait_for(lock, config_.frame_wait);
            }
            if (!running_)
                break;

            frames_.update();
            if (!frames_.hasNewData())
            {
                frames_.cleanup();
                continue;
            }

            const Frame* frame = frames_.getData();
            if (!frame || !frame->valid())
            {
                frames_.cleanup();
                continue;
            }

            if (!write_frame(client, *frame))
            {
                log->warn("write_frame failed; client disconnected");
                frames_.cleanup();
                break;
            }

            snapshot.bytes_sent += frame_header_wire_size + frame->pixels.size();
            snapshot.frames_sent++;
            deliver_stats(snapshot);
            frames_.cleanup();

            if (snapshot.frames_sent % 300 == 0)
                log->debug(
                    "Sent {} frames ({:.2f} MB)", snapshot.frames_sent, static_cast<double>(snapshot.bytes_sent) / 1e6);
        }

        client.close();
        {
            std::lock_guard<std::mutex> lock(state_mutex_);
            if (client_ == &client)
                client_ = nullptr;
        }

        log->info("Client disconnected");
        deliver_stats(ServerStats{});
    }

    {
        std::lock_guard<std::mutex> lock(state_mutex_);
        listener_ = nullptr;
    }
    log->info("Net thread exited");
}

} // namespace stream
