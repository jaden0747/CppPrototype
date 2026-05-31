#pragma once

#include "stream/input_protocol.hpp"

#include <cstdint>
#include <string>

namespace stream
{

// ---------------------------------------------------------------------------
// InputSender — sends input events from client to server via UDP
//
// Thin wrapper around UdpSender. Each event is serialized as JSON and sent
// as a single UDP datagram. No reliability needed — input events are
// idempotent and high-frequency (dropped mouse moves are irrelevant).
// ---------------------------------------------------------------------------

class InputSender
{
public:
    InputSender() = default;
    ~InputSender();

    InputSender(const InputSender&)            = delete;
    InputSender& operator=(const InputSender&) = delete;

    bool open(const std::string& server_ip, uint16_t port = INPUT_PORT);
    void close();
    bool is_open() const;

    bool send_key(const std::string& event_type, const std::string& key);
    bool send_mouse_move(int x, int y);
    bool send_mouse_click(int x, int y, const std::string& button, bool pressed);
    bool send_event(const InputEvent& event);

private:
    class UdpSender* m_sender = nullptr;
};

} // namespace stream
