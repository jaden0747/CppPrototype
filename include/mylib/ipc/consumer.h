#pragma once

#include "mylib/ipc/channel.h"

#include <string>
#include <vector>

// Non-blocking consumer intended to be called from the main (render) thread.
// drain() pulls all available lines out of the channel without blocking.
class Consumer
{
public:
    explicit Consumer(IChannel& channel);
    ~Consumer();

    bool open();  // call once before the first drain()
    void close(); // closing the read end sends EPIPE to the producer

    // Returns all complete newline-terminated messages available right now.
    // Partial lines are buffered internally and completed on the next call.
    std::vector<std::string> drain();

private:
    IChannel&   m_channel;
    std::string m_partial; // accumulates bytes that didn't end with '\n' yet
    bool        m_open = false;
};
