#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>

namespace stream
{

class TcpConnection
{
public:
    TcpConnection();
    ~TcpConnection();

    TcpConnection(TcpConnection&& other) noexcept;
    TcpConnection& operator=(TcpConnection&& other) noexcept;

    TcpConnection(const TcpConnection&)            = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    bool read_exact(void* dst, size_t size);
    bool write_exact(const void* src, size_t size);
    void close();
    bool is_open() const;

private:
    struct Impl;
    explicit TcpConnection(std::unique_ptr<Impl> impl);

    std::unique_ptr<Impl> impl_;

    friend class TcpListener;
    friend std::optional<TcpConnection> connect_tcp(const std::string&, uint16_t);
};

class TcpListener
{
public:
    explicit TcpListener(uint16_t port);
    ~TcpListener();

    TcpListener(TcpListener&& other) noexcept;
    TcpListener& operator=(TcpListener&& other) noexcept;

    TcpListener(const TcpListener&)            = delete;
    TcpListener& operator=(const TcpListener&) = delete;

    std::optional<TcpConnection> accept_for(std::chrono::milliseconds timeout);
    void                         close();
    bool                         is_open() const;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

std::optional<TcpConnection> connect_tcp(const std::string& host, uint16_t port);
void                         ignore_sigpipe();

} // namespace stream
