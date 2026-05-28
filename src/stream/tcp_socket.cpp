#include "stream/tcp_socket.hpp"

#include <asio.hpp>

#ifndef _WIN32
#include <signal.h>
#endif

#include <thread>

namespace stream
{

using asio::ip::tcp;

struct TcpConnection::Impl
{
    asio::io_context io;
    tcp::socket      socket{io};
};

struct TcpListener::Impl
{
    asio::io_context io;
    tcp::acceptor    acceptor{io};
};

TcpConnection::TcpConnection() = default;

TcpConnection::TcpConnection(std::unique_ptr<Impl> impl)
    : m_impl(std::move(impl))
{
}

TcpConnection::~TcpConnection()
{
    close();
}

TcpConnection::TcpConnection(TcpConnection&& other) noexcept            = default;
TcpConnection& TcpConnection::operator=(TcpConnection&& other) noexcept = default;

bool TcpConnection::read_exact(void* dst, size_t size)
{
    if (!m_impl || !m_impl->socket.is_open())
        return false;

    asio::error_code ec;
    asio::read(m_impl->socket, asio::buffer(dst, size), asio::transfer_exactly(size), ec);
    return !ec;
}

bool TcpConnection::write_exact(const void* src, size_t size)
{
    if (!m_impl || !m_impl->socket.is_open())
        return false;

    asio::error_code ec;
    asio::write(m_impl->socket, asio::buffer(src, size), asio::transfer_exactly(size), ec);
    return !ec;
}

void TcpConnection::close()
{
    if (!m_impl || !m_impl->socket.is_open())
        return;

    asio::error_code ignored;
    m_impl->socket.shutdown(tcp::socket::shutdown_both, ignored);
    m_impl->socket.close(ignored);
}

bool TcpConnection::is_open() const
{
    return m_impl && m_impl->socket.is_open();
}

TcpListener::TcpListener(uint16_t port)
    : m_impl(std::make_unique<Impl>())
{
    asio::error_code ec;
    tcp::endpoint    endpoint(tcp::v4(), port);
    m_impl->acceptor.open(endpoint.protocol(), ec);
    if (ec)
        return;

    m_impl->acceptor.set_option(tcp::acceptor::reuse_address(true), ec);
    m_impl->acceptor.bind(endpoint, ec);
    if (ec)
    {
        close();
        return;
    }

    m_impl->acceptor.listen(asio::socket_base::max_listen_connections, ec);
    if (ec)
    {
        close();
        return;
    }

    m_impl->acceptor.non_blocking(true, ec);
}

TcpListener::~TcpListener()
{
    close();
}

TcpListener::TcpListener(TcpListener&& other) noexcept            = default;
TcpListener& TcpListener::operator=(TcpListener&& other) noexcept = default;

std::optional<TcpConnection> TcpListener::accept_for(std::chrono::milliseconds timeout)
{
    if (!m_impl || !m_impl->acceptor.is_open())
        return std::nullopt;

    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        auto             conn = std::unique_ptr<TcpConnection::Impl>(new TcpConnection::Impl());
        asio::error_code ec;
        m_impl->acceptor.accept(conn->socket, ec);
        if (!ec)
            return TcpConnection(std::move(conn));
        if (ec != asio::error::would_block && ec != asio::error::try_again)
            return std::nullopt;
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return std::nullopt;
}

void TcpListener::close()
{
    if (!m_impl || !m_impl->acceptor.is_open())
        return;

    asio::error_code ignored;
    m_impl->acceptor.close(ignored);
}

bool TcpListener::is_open() const
{
    return m_impl && m_impl->acceptor.is_open();
}

std::optional<TcpConnection> connect_tcp(const std::string& host, uint16_t port)
{
    auto          conn = std::unique_ptr<TcpConnection::Impl>(new TcpConnection::Impl());
    tcp::resolver resolver(conn->io);

    asio::error_code ec;
    const auto       endpoints = resolver.resolve(host, std::to_string(port), ec);
    if (ec)
        return std::nullopt;

    asio::connect(conn->socket, endpoints, ec);
    if (ec)
        return std::nullopt;

    return TcpConnection(std::move(conn));
}

void ignore_sigpipe()
{
#ifndef _WIN32
    ::signal(SIGPIPE, SIG_IGN);
#endif
}

} // namespace stream
