#include "stream/gl_frame_capture.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <utility>

namespace stream
{

namespace
{

void configure_texture()
{
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

} // namespace

GlFrameCapture::GlFrameCapture(dc::SenderPort<Frame>& frames)
    : frames_(frames)
{
    glGenTextures(1, &preview_texture_);
    glBindTexture(GL_TEXTURE_2D, preview_texture_);
    configure_texture();
}

GlFrameCapture::~GlFrameCapture()
{
    if (preview_texture_ != 0)
        glDeleteTextures(1, &preview_texture_);
}

bool GlFrameCapture::capture(int framebuffer_width, int framebuffer_height)
{
    if (framebuffer_width <= 0 || framebuffer_height <= 0)
        return false;

    Frame* slot = frames_.reserve();
    if (!slot)
        return false;

    slot->width       = framebuffer_width;
    slot->height      = framebuffer_height;
    slot->format      = PixelFormat::Rgb8;
    slot->compression = Compression::None;
    slot->pixels.resize(static_cast<size_t>(framebuffer_width) * static_cast<size_t>(framebuffer_height) * 3u);

    glReadPixels(0, 0, framebuffer_width, framebuffer_height, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data());

    const size_t stride = static_cast<size_t>(framebuffer_width) * 3u;
    for (int row = 0; row < framebuffer_height / 2; ++row)
    {
        auto beg = slot->pixels.begin();
        std::swap_ranges(
            beg + static_cast<size_t>(row) * stride,
            beg + static_cast<size_t>(row + 1) * stride,
            beg + static_cast<size_t>(framebuffer_height - 1 - row) * stride);
    }

    glBindTexture(GL_TEXTURE_2D, preview_texture_);
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGB,
        framebuffer_width,
        framebuffer_height,
        0,
        GL_RGB,
        GL_UNSIGNED_BYTE,
        slot->pixels.data());
    preview_ready_  = true;
    preview_width_  = framebuffer_width;
    preview_height_ = framebuffer_height;

    frames_.deliver();
    if (after_deliver_)
        after_deliver_();

    return true;
}

void GlFrameCapture::set_after_deliver(std::function<void()> callback)
{
    after_deliver_ = std::move(callback);
}

uint32_t GlFrameCapture::preview_texture() const
{
    return preview_texture_;
}

int GlFrameCapture::preview_width() const
{
    return preview_width_;
}

int GlFrameCapture::preview_height() const
{
    return preview_height_;
}

bool GlFrameCapture::preview_ready() const
{
    return preview_ready_;
}

} // namespace stream
