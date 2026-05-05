#pragma once

#include "mylib/ipc/channel.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>
#include <thread>

// Runs a background thread that calls msgFn() on each tick and writes the
// result to the channel. Swap the channel or the message function independently.
class Producer
{
public:
    using MessageFn = std::function<std::string()>;

    Producer(IChannel& channel, MessageFn msgFn, std::chrono::milliseconds interval = std::chrono::milliseconds{500});
    ~Producer();

    void start();
    void stop(); // idempotent; waits for thread to exit

    // Built-in data sources — pass one to the constructor or roll your own lambda.
    static MessageFn sensorSource(); // timestamped sensor_temp readings (Day 1)

private:
    void run();

    IChannel&                 m_channel;
    MessageFn                 m_msgFn;
    std::chrono::milliseconds m_interval;

    std::atomic<bool>       m_running{false};
    std::thread             m_thread;
    std::mutex              m_sleepMtx;
    std::condition_variable m_sleepCv; // lets stop() interrupt the inter-message sleep
};
