#include "stream/input_receiver.hpp"

#include "mylib/log.hpp"
#include "stream/udp_socket.hpp"

#include <cstring>

namespace stream
{

InputReceiver::~InputReceiver()
{
    stop();
}

void InputReceiver::set_on_event(InputEventCallback cb)
{
    m_callback = std::move(cb);
}

void InputReceiver::start(uint16_t port)
{
    if (m_running.exchange(true))
        return;
    m_port   = port;
    m_thread = std::thread(&InputReceiver::run, this);
}

void InputReceiver::stop()
{
    m_running.store(false);
    if (m_thread.joinable())
        m_thread.join();
}

void InputReceiver::run()
{
    auto log = Log::get("input");
    log->info("Input receiver listening on UDP :{}", m_port);

    UdpReceiver receiver;
    if (!receiver.bind(m_port))
    {
        log->error("Failed to bind input port {}", m_port);
        m_running.store(false);
        return;
    }

    // 1 KB buffer — more than enough for any JSON input event
    uint8_t buf[1024];

    while (m_running)
    {
        int n = receiver.receive(buf, sizeof(buf));
        if (n < 0)
            break; // hard error
        if (n == 0)
            continue; // timeout, loop and check m_running

        std::string json_str(reinterpret_cast<const char*>(buf), static_cast<size_t>(n));
        InputEvent  event;
        if (!InputEvent::deserialize(json_str, event))
        {
            log->warn("Failed to parse input event: {}", json_str);
            continue;
        }

        m_events_received.fetch_add(1);

        if (m_callback)
            m_callback(event);
    }

    receiver.close();
    log->info("Input receiver stopped");
    m_running.store(false);
}

} // namespace stream
