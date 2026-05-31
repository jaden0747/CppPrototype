#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

namespace stream
{

// ---------------------------------------------------------------------------
// UdpSender — unicast UDP socket for fire-and-forget packet sending.
// ---------------------------------------------------------------------------
class UdpSender
{
public:
    UdpSender();
    ~UdpSender();

    UdpSender(const UdpSender&)            = delete;
    UdpSender& operator=(const UdpSender&) = delete;

    bool open(const std::string& dest_ip, uint16_t dest_port);
    bool send(const void* data, size_t len);
    void close();
    bool is_open() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

// ---------------------------------------------------------------------------
// UdpReceiver — bound UDP socket for blocking receive with a 100 ms timeout.
//
// receive() returns the number of bytes placed in buf (>0), 0 on timeout, or
// -1 on a hard error (e.g. socket closed).  Callers should loop on 0 and
// exit on -1 or when a running flag is cleared.
// ---------------------------------------------------------------------------
class UdpReceiver
{
public:
    UdpReceiver();
    ~UdpReceiver();

    UdpReceiver(const UdpReceiver&)            = delete;
    UdpReceiver& operator=(const UdpReceiver&) = delete;

    bool bind(uint16_t port);
    int  receive(void* buf, size_t max_len); // returns bytes, 0=timeout, -1=error
    void close();
    bool is_open() const;

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace stream
