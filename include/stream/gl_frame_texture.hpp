#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"

#include <cstdint>
#include <functional>

namespace stream
{

class GlFrameTexture
{
public:
    GlFrameTexture();
    ~GlFrameTexture();

    GlFrameTexture(const GlFrameTexture&)            = delete;
    GlFrameTexture& operator=(const GlFrameTexture&) = delete;

    bool update(dc::ReceiverPort<Frame>& frames);

    // Optional decode hook: called when a compressed frame arrives.
    // Writes decoded raw RGB pixels into rgb_out; returns false on failure.
    void set_decode_fn(std::function<bool(const Frame&, Frame&)> fn);

    uint32_t texture() const;
    int      width() const;
    int      height() const;
    bool     ready() const;

private:
    void upload_rgb(const Frame& frame);

    uint32_t                                  m_texture = 0;
    int                                       m_width   = 0;
    int                                       m_height  = 0;
    bool                                      m_ready   = false;
    std::function<bool(const Frame&, Frame&)> m_decode_fn;
    Frame                                     m_decode_buf;
};

} // namespace stream
