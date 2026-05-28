#include "stream/stream_stats.hpp"

namespace stream
{

FpsCounter::FpsCounter(float window_seconds)
    : m_window(window_seconds)
{
}

float FpsCounter::tick()
{
    const auto now = clock::now();
    m_stamps.push_back(now);

    // Drop timestamps older than the window.
    const auto cutoff = now - std::chrono::duration<float>(m_window);
    while (!m_stamps.empty() && m_stamps.front() < cutoff)
        m_stamps.pop_front();

    // Need at least two timestamps to compute a rate.
    if (m_stamps.size() < 2)
        return m_fps;

    const float elapsed = std::chrono::duration<float>(m_stamps.back() - m_stamps.front()).count();
    if (elapsed > 0.0f)
        m_fps = static_cast<float>(m_stamps.size() - 1) / elapsed;

    return m_fps;
}

float FpsCounter::value() const
{
    return m_fps;
}

} // namespace stream
