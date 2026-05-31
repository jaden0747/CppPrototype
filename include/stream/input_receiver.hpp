#pragma once

#include "stream/input_protocol.hpp"

#include <atomic>
#include <cstdint>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace stream
{

// ---------------------------------------------------------------------------
// InputReceiver — listens for input events on a UDP port (server-side)
//
// Runs a background thread that receives UDP datagrams, deserializes them
// as InputEvent JSON, and dispatches to a registered callback. The callback
// is invoked on the receiver thread — keep it fast (just queue/log).
// ---------------------------------------------------------------------------

using InputEventCallback = std::function<void(const InputEvent&)>;

class InputReceiver
{
public:
    InputReceiver() = default;
    ~InputReceiver();

    InputReceiver(const InputReceiver&)            = delete;
    InputReceiver& operator=(const InputReceiver&) = delete;

    void set_on_event(InputEventCallback cb);

    void start(uint16_t port = INPUT_PORT);
    void stop();
    bool is_running() const { return m_running.load(); }

    // Thread-safe access to recent events (for UI display)
    uint64_t events_received() const { return m_events_received.load(); }

private:
    void run();

    InputEventCallback    m_callback;
    std::atomic<bool>     m_running{false};
    std::atomic<uint64_t> m_events_received{0};
    uint16_t              m_port = INPUT_PORT;
    std::thread           m_thread;
};

} // namespace stream
