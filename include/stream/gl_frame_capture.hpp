#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"

#include <cstdint>
#include <functional>

namespace stream
{

class GlFrameCapture
{
public:
    explicit GlFrameCapture(dc::SenderPort<Frame>& frames);
    ~GlFrameCapture();

    GlFrameCapture(const GlFrameCapture&)            = delete;
    GlFrameCapture& operator=(const GlFrameCapture&) = delete;

    bool capture(int framebuffer_width, int framebuffer_height);
    void set_after_deliver(std::function<void()> callback);

    // Optional encode hook: called with the raw RGB frame after glReadPixels.
    // The callback may transform frame.pixels in-place (e.g. RGB → H264 NAL bytes).
    // Return false to skip delivery (frame not sent to network).
    void set_encode_fn(std::function<bool(Frame&)> fn);

    // GPU-blit downscale before glReadPixels (retina fix).
    // When set and different from the framebuffer size, blits the default
    // framebuffer to an internal FBO at this size before reading pixels.
    // Call once after window creation with the logical (non-HiDPI) window size.
    void set_target_size(int width, int height);

    uint32_t preview_texture() const;
    int      preview_width() const;
    int      preview_height() const;
    bool     preview_ready() const;

private:
    void ensure_blit_fbo(int w, int h);

    dc::SenderPort<Frame>&      m_frames;
    std::function<void()>       m_after_deliver;
    std::function<bool(Frame&)> m_encode_fn;

    // Preview texture (shown in ImGui before encoding)
    uint32_t m_preview_texture = 0;
    int      m_preview_width   = 0;
    int      m_preview_height  = 0;
    bool     m_preview_ready   = false;

    // FBO for GPU-side downscale (retina: 2× physical → logical resolution)
    int      m_target_width  = 0;
    int      m_target_height = 0;
    uint32_t m_fbo           = 0;
    uint32_t m_fbo_rbo       = 0;
    int      m_fbo_width     = 0;
    int      m_fbo_height    = 0;
};

} // namespace stream
