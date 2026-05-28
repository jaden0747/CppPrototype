#pragma once

#include <chrono>
#include <limits>

namespace stream
{

class GlApp;

class GlLayer
{
public:
    virtual ~GlLayer() = default;

    virtual void onAttach(GlApp&)
    {
    }
    virtual void onDetach()
    {
    }
    virtual void onUpdate(float /*dt*/)
    {
    }
    virtual void onImGui()
    {
    }
    virtual void onRender(int /*fbW*/, int /*fbH*/)
    {
    }

    virtual bool onKey(int /*key*/, int /*scancode*/, int /*action*/, int /*mods*/)
    {
        return false;
    }
    virtual bool onMouseButton(int /*button*/, int /*action*/, int /*mods*/)
    {
        return false;
    }
    virtual bool onScroll(double /*xoff*/, double /*yoff*/)
    {
        return false;
    }
    virtual void onResize(int /*fbW*/, int /*fbH*/)
    {
    }

    // Return the earliest time this layer needs the render loop to wake up.
    // Default = max() means "no preference; use GlApp's max_render_fps."
    // Capture layers override this to drive precise frame timing without spinning.
    virtual std::chrono::steady_clock::time_point nextWakeup() const
    {
        return std::chrono::steady_clock::time_point::max();
    }
};

} // namespace stream
