#pragma once

#include <chrono>
#include <cstdint>
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

class FpsCounter
{
public:
    explicit FpsCounter(float alpha = 0.1f);
    float tick();
    float value() const;

private:
    using clock = std::chrono::steady_clock;

    float             alpha_ = 0.1f;
    float             fps_   = 0.0f;
    clock::time_point last_  = clock::now();
};

} // namespace stream
