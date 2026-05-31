#include "stream/udp_socket.hpp"

#include <asio.hpp>

#ifdef _WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <sys/time.h>
#endif

namespace stream
{

using asio::ip::udp;

// ---------------------------------------------------------------------------
// UdpSender
// ---------------------------------------------------------------------------

struct UdpSender::Impl
{
    asio::io_context io;
    udp::socket      socket{io};
    udp::endpoint    endpoint;
};

UdpSender::UdpSender()  = default;
UdpSender::~UdpSender() { close(); }

bool UdpSender::open(const std::string& dest_ip, uint16_t dest_port)
{
    m_impl = std::make_unique<Impl>();

    asio::error_code ec;
    m_impl->endpoint = udp::endpoint(asio::ip::make_address(dest_ip, ec), dest_port);
    if (ec)
        return false;

    m_impl->socket.open(udp::v4(), ec);
    return !ec;
}

bool UdpSender::send(const void* data, size_t len)
{
    if (!m_impl || !m_impl->socket.is_open())
        return false;

    asio::error_code ec;
    m_impl->socket.send_to(asio::buffer(data, len), m_impl->endpoint, 0, ec);
    return !ec;
}

void UdpSender::close()
{
    if (!m_impl || !m_impl->socket.is_open())
        return;
    asio::error_code ignored;
    m_impl->socket.close(ignored);
}

bool UdpSender::is_open() const
{
    return m_impl && m_impl->socket.is_open();
}

// ---------------------------------------------------------------------------
// UdpReceiver
// ---------------------------------------------------------------------------

struct UdpReceiver::Impl
{
    asio::io_context io;
    udp::socket      socket{io};
};

UdpReceiver::UdpReceiver()  = default;
UdpReceiver::~UdpReceiver() { close(); }

bool UdpReceiver::bind(uint16_t port)
{
    m_impl = std::make_unique<Impl>();

    asio::error_code ec;
    m_impl->socket.open(udp::v4(), ec);
    if (ec)
        return false;

    m_impl->socket.set_option(asio::socket_base::reuse_address(true), ec);
    m_impl->socket.bind(udp::endpoint(udp::v4(), port), ec);
    if (ec)
        return false;

    // Set a 100 ms receive timeout so the receive loop can check m_running.
#ifdef _WIN32
    DWORD timeout_ms = 100;
    setsockopt(
        m_impl->socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO,
        reinterpret_cast<const char*>(&timeout_ms), sizeof(timeout_ms));
#else
    struct timeval tv;
    tv.tv_sec  = 0;
    tv.tv_usec = 100 * 1000; // 100 ms
    setsockopt(m_impl->socket.native_handle(), SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
#endif

    return true;
}

int UdpReceiver::receive(void* buf, size_t max_len)
{
    if (!m_impl || !m_impl->socket.is_open())
        return -1;

    udp::endpoint    sender;
    asio::error_code ec;
    const size_t     n = m_impl->socket.receive_from(asio::buffer(buf, max_len), sender, 0, ec);

    if (!ec)
        return static_cast<int>(n);

    // Timeout / would-block: treat as "no data yet"
    if (ec == asio::error::timed_out || ec == asio::error::would_block || ec == asio::error::try_again)
        return 0;

    // Hard error (socket closed, network down, etc.)
    return -1;
}

void UdpReceiver::close()
{
    if (!m_impl || !m_impl->socket.is_open())
        return;
    asio::error_code ignored;
    m_impl->socket.close(ignored);
}

bool UdpReceiver::is_open() const
{
    return m_impl && m_impl->socket.is_open();
}

} // namespace stream
