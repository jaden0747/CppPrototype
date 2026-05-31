#include "stream/gl_frame_texture.hpp"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

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
    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);
    configure_texture();
}

GlFrameTexture::~GlFrameTexture()
{
    if (m_texture != 0)
        glDeleteTextures(1, &m_texture);
}

void GlFrameTexture::set_decode_fn(std::function<bool(const Frame&, Frame&)> fn)
{
    m_decode_fn = std::move(fn);
}

void GlFrameTexture::upload_rgb(const Frame& frame)
{
    glBindTexture(GL_TEXTURE_2D, m_texture);
    if (!m_ready || frame.width != m_width || frame.height != m_height)
    {
        glTexImage2D(
            GL_TEXTURE_2D, 0, GL_RGB, frame.width, frame.height, 0, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels.data());
    }
    else
    {
        glTexSubImage2D(
            GL_TEXTURE_2D, 0, 0, 0, frame.width, frame.height, GL_RGB, GL_UNSIGNED_BYTE, frame.pixels.data());
    }
    m_width  = frame.width;
    m_height = frame.height;
    m_ready  = true;
}

bool GlFrameTexture::update(dc::ReceiverPort<Frame>& frames)
{
    bool changed = false;

    frames.update();
    if (frames.hasNewData())
    {
        const Frame* frame = frames.getData();
        if (frame && !frame->pixels.empty())
        {
            if (frame->compression == Compression::H264 && m_decode_fn)
            {
                if (m_decode_fn(*frame, m_decode_buf))
                {
                    upload_rgb(m_decode_buf);
                    changed = true;
                }
            }
            else if (frame->compression == Compression::None && frame->format == PixelFormat::Rgb8)
            {
                upload_rgb(*frame);
                changed = true;
            }
        }
    }
    frames.cleanup();

    return changed;
}

uint32_t GlFrameTexture::texture() const
{
    return m_texture;
}

int GlFrameTexture::width() const
{
    return m_width;
}

int GlFrameTexture::height() const
{
    return m_height;
}

bool GlFrameTexture::ready() const
{
    return m_ready;
}

} // namespace stream
