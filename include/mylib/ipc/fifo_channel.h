#pragma once

#include "mylib/ipc/channel.h"
#include <string>

// Day 1 — Named pipe (FIFO) implementation of IChannel.
//
// create()      : mkfifo(path)
// openWriter()  : open(O_WRONLY) — blocks until reader end is open
// openReader()  : open(O_RDONLY | O_NONBLOCK) — returns immediately
// tryRead()     : read(); returns -1/EAGAIN when no data (non-blocking)
// destroy()     : close both ends, unlink path
class FifoChannel final : public IChannel
{
public:
    explicit FifoChannel(std::string path);

    void create() override;
    void destroy() override;

    bool    openWriter() override;
    ssize_t write(const void* buf, size_t n) override;
    void    closeWriter() override;

    bool    openReader() override;
    ssize_t tryRead(void* buf, size_t n) override;
    void    closeReader() override;

private:
    std::string m_path;
    int         m_writerFd = -1;
    int         m_readerFd = -1;
};
