#include "stream/stream_stats.hpp"

namespace stream
{

FpsCounter::FpsCounter(float alpha)
    : alpha_(alpha)
{
}

float FpsCounter::tick()
{
    const auto  now = clock::now();
    const float dt  = std::chrono::duration<float>(now - last_).count();
    last_           = now;
    if (dt > 0.0f)
        fps_ = fps_ * (1.0f - alpha_) + (1.0f / dt) * alpha_;
    return fps_;
}

float FpsCounter::value() const
{
    return fps_;
}

} // namespace stream
