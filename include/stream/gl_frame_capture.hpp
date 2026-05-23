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

    uint32_t preview_texture() const;
    int      preview_width() const;
    int      preview_height() const;
    bool     preview_ready() const;

private:
    dc::SenderPort<Frame>& frames_;
    std::function<void()>  after_deliver_;
    uint32_t               preview_texture_ = 0;
    int                    preview_width_   = 0;
    int                    preview_height_  = 0;
    bool                   preview_ready_   = false;
};

} // namespace stream
