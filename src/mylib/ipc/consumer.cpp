#include "mylib/ipc/consumer.h"

#include <cerrno>
#include <cstring>

Consumer::Consumer(IChannel& channel) : m_channel(channel) {}

Consumer::~Consumer()
{
    close();
}

bool Consumer::open()
{
    m_open = m_channel.openReader();
    return m_open;
}

void Consumer::close()
{
    if (m_open)
    {
        m_channel.closeReader();
        m_open = false;
    }
}

std::vector<std::string> Consumer::drain()
{
    std::vector<std::string> lines;
    if (!m_open)
        return lines;

    char    buf[4096];
    ssize_t n;
    while ((n = m_channel.tryRead(buf, sizeof(buf) - 1)) > 0)
    {
        buf[n]            = '\0';
        const char* start = buf;
        const char* end   = buf + n;

        while (start < end)
        {
            const char* nl = static_cast<const char*>(::memchr(start, '\n', end - start));
            if (nl)
            {
                lines.push_back(m_partial + std::string(start, nl));
                m_partial.clear();
                start = nl + 1;
            }
            else
            {
                m_partial.append(start, end);
                break;
            }
        }
    }
    // n == -1 with errno == EAGAIN/EWOULDBLOCK is normal — channel is empty

    return lines;
}
