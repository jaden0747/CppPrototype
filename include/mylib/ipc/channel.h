#pragma once

#include <cstddef>
#include <sys/types.h>

// Transport abstraction for POSIX IPC mechanisms.
//
// Implementations: FifoChannel (Day 1), ShmChannel (Day 2), SemChannel (Day 3).
// Producer and Consumer talk to this interface only — swap the channel, keep
// everything else.
//
// Threading contract:
//   openWriter / write / closeWriter  — called exclusively from the producer thread
//   openReader / tryRead / closeReader — called exclusively from the consumer thread
//   create / destroy                  — called from the owning thread before start
class IChannel
{
public:
    virtual ~IChannel() = default;

    // One-time setup/teardown of the OS-level resource (mkfifo, shm_open, …).
    virtual void create()  = 0;
    virtual void destroy() = 0;

    // Producer side
    virtual bool    openWriter()                          = 0;
    virtual ssize_t write(const void* buf, size_t n)     = 0;
    virtual void    closeWriter()                         = 0;

    // Consumer side — tryRead must never block (O_NONBLOCK, EAGAIN is normal)
    virtual bool    openReader()                          = 0;
    virtual ssize_t tryRead(void* buf, size_t n)          = 0;
    virtual void    closeReader()                         = 0;
};
