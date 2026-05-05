#include "mylib/ipc/producer.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <ctime>

Producer::Producer(IChannel& channel, MessageFn msgFn, std::chrono::milliseconds interval)
    : m_channel(channel)
    , m_msgFn(std::move(msgFn))
    , m_interval(interval)
{
}

Producer::~Producer()
{
    stop();
}

void Producer::start()
{
    m_running.store(true);
    m_thread = std::thread(&Producer::run, this);
}

void Producer::stop()
{
    if (!m_running.exchange(false))
        return;             // already stopped
    m_sleepCv.notify_one(); // interrupt inter-message sleep immediately
    if (m_thread.joinable())
        m_thread.join();
}

Producer::MessageFn Producer::sensorSource()
{
    // Capture seq by value so multiple Producer instances get independent counters.
    return [seq = 0L]() mutable -> std::string
    {
        struct timespec ts;
        ::clock_gettime(CLOCK_REALTIME, &ts);
        char buf[256];
        ::snprintf(
            buf,
            sizeof(buf),
            "[%ld.%03ld] sensor_temp=%.2f  seq=%ld",
            (long)ts.tv_sec,
            ts.tv_nsec / 1'000'000L,
            20.0 + (::rand() % 100) / 10.0,
            seq++);
        return buf;
    };
}

void Producer::run()
{
    if (!m_channel.openWriter())
        return;

    while (m_running.load())
    {
        std::string msg = m_msgFn();
        msg += '\n'; // consumer splits on newlines

        if (m_channel.write(msg.data(), msg.size()) < 0)
        {
            if (errno != EPIPE)
                ::perror("Producer::run write");
            break;
        }

        // Interruptible sleep: stop() notifies the cv so shutdown is immediate.
        std::unique_lock<std::mutex> lk(m_sleepMtx);
        m_sleepCv.wait_for(lk, m_interval, [this] { return !m_running.load(); });
    }

    m_channel.closeWriter();
}
