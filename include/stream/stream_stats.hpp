#pragma once

#include <chrono>
#include <cstdint>
#include <deque>
#include <string>

namespace stream
{

struct ServerStats
{
    uint64_t    bytes_sent  = 0;
    uint64_t    frames_sent = 0;
    bool        connected   = false;
    std::string peer;
};

struct ClientStats
{
    uint64_t    bytes_received  = 0;
    uint64_t    frames_received = 0;
    float       fps             = 0.0f;
    bool        connected       = false;
    std::string status          = "Disconnected";
};

// Sliding-window FPS counter: reports frames / elapsed over the last `window`
// seconds. Accurate from the second frame onward; no warm-up lag.
class FpsCounter
{
public:
    explicit FpsCounter(float window_seconds = 1.0f);
    float tick();
    float value() const;

private:
    using clock      = std::chrono::steady_clock;
    using time_point = clock::time_point;

    float                  m_window;
    float                  m_fps = 0.0f;
    std::deque<time_point> m_stamps;
};

} // namespace stream
