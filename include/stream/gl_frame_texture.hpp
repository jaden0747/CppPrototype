#pragma once

#include "mylib/data_container.hpp"
#include "stream/frame.hpp"

#include <cstdint>

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

    uint32_t texture() const;
    int      width() const;
    int      height() const;
    bool     ready() const;

private:
    uint32_t texture_ = 0;
    int      width_   = 0;
    int      height_  = 0;
    bool     ready_   = false;
};

} // namespace stream
