#include "stream/gl_frame_texture.hpp"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

GlFrameTexture::GlFrameTexture()
{
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    configure_texture();
}

GlFrameTexture::~GlFrameTexture()
{
    if (texture_ != 0)
        glDeleteTextures(1, &texture_);
}

bool GlFrameTexture::update(dc::ReceiverPort<Frame>& frames)
{
    bool changed = false;

    frames.update();
    if (frames.hasNewData())
    {
        const Frame* frame = frames.getData();
        if (frame && frame->compression == Compression::None && frame->format == PixelFormat::Rgb8 &&
            !frame->pixels.empty())
        {
            glBindTexture(GL_TEXTURE_2D, texture_);
            if (!ready_ || frame->width != width_ || frame->height != height_)
            {
                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RGB,
                    frame->width,
                    frame->height,
                    0,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    frame->pixels.data());
            }
            else
            {
                glTexSubImage2D(
                    GL_TEXTURE_2D,
                    0,
                    0,
                    0,
                    frame->width,
                    frame->height,
                    GL_RGB,
                    GL_UNSIGNED_BYTE,
                    frame->pixels.data());
            }
            width_  = frame->width;
            height_ = frame->height;
            ready_  = true;
            changed = true;
        }
    }
    frames.cleanup();

    return changed;
}

uint32_t GlFrameTexture::texture() const
{
    return texture_;
}

int GlFrameTexture::width() const
{
    return width_;
}

int GlFrameTexture::height() const
{
    return height_;
}

bool GlFrameTexture::ready() const
{
    return ready_;
}

} // namespace stream
