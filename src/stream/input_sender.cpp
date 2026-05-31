#include "stream/input_sender.hpp"

#include "mylib/log.hpp"
#include "stream/udp_socket.hpp"

namespace stream
{

InputSender::~InputSender()
{
    close();
}

bool InputSender::open(const std::string& server_ip, uint16_t port)
{
    close();
    m_sender = new UdpSender();
    if (!m_sender->open(server_ip, port))
    {
        delete m_sender;
        m_sender = nullptr;
        return false;
    }
    Log::get("input")->info("Input sender opened -> {}:{}", server_ip, port);
    return true;
}

void InputSender::close()
{
    if (m_sender)
    {
        m_sender->close();
        delete m_sender;
        m_sender = nullptr;
    }
}

bool InputSender::is_open() const
{
    return m_sender && m_sender->is_open();
}

bool InputSender::send_event(const InputEvent& event)
{
    if (!m_sender)
        return false;
    std::string payload = event.serialize();
    return m_sender->send(payload.data(), payload.size());
}

bool InputSender::send_key(const std::string& event_type, const std::string& key)
{
    return send_event(InputEvent::from_key(event_type, key));
}

bool InputSender::send_mouse_move(int x, int y)
{
    return send_event(InputEvent::from_mouse_move(x, y));
}

bool InputSender::send_mouse_click(int x, int y, const std::string& button, bool pressed)
{
    return send_event(InputEvent::from_mouse_click(x, y, button, pressed));
}

} // namespace stream
