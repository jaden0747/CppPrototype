#include "mylib/ipc/fifo_channel.h"

#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

FifoChannel::FifoChannel(std::string path)
    : m_path(std::move(path))
{
}

void FifoChannel::create()
{
    ::unlink(m_path.c_str()); // remove stale file from a previous run
    if (::mkfifo(m_path.c_str(), 0666) < 0)
        ::perror("FifoChannel::create");
}

void FifoChannel::destroy()
{
    closeWriter();
    closeReader();
    ::unlink(m_path.c_str());
}

bool FifoChannel::openWriter()
{
    // Blocks until the read end is open — call after openReader() returns.
    m_writerFd = ::open(m_path.c_str(), O_WRONLY);
    if (m_writerFd < 0)
    {
        ::perror("FifoChannel::openWriter");
        return false;
    }
    return true;
}

ssize_t FifoChannel::write(const void* buf, size_t n)
{
    return ::write(m_writerFd, buf, n);
}

void FifoChannel::closeWriter()
{
    if (m_writerFd >= 0)
    {
        ::close(m_writerFd);
        m_writerFd = -1;
    }
}

bool FifoChannel::openReader()
{
    // O_NONBLOCK: returns immediately even if no writer is present yet.
    // The producer's openWriter() will unblock once this returns.
    m_readerFd = ::open(m_path.c_str(), O_RDONLY | O_NONBLOCK);
    if (m_readerFd < 0)
    {
        ::perror("FifoChannel::openReader");
        return false;
    }
    return true;
}

ssize_t FifoChannel::tryRead(void* buf, size_t n)
{
    return ::read(m_readerFd, buf, n);
}

void FifoChannel::closeReader()
{
    if (m_readerFd >= 0)
    {
        ::close(m_readerFd);
        m_readerFd = -1;
    }
}
