#include "stream/gl_frame_capture.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

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
    : m_frames(frames)
{
    glGenTextures(1, &m_preview_texture);
    glBindTexture(GL_TEXTURE_2D, m_preview_texture);
    configure_texture();
}

GlFrameCapture::~GlFrameCapture()
{
    if (m_fbo != 0)
    {
        glDeleteFramebuffers(1, &m_fbo);
        glDeleteRenderbuffers(1, &m_fbo_rbo);
    }
    if (m_preview_texture != 0)
        glDeleteTextures(1, &m_preview_texture);
}

void GlFrameCapture::ensure_blit_fbo(int w, int h)
{
    if (m_fbo != 0 && m_fbo_width == w && m_fbo_height == h)
        return;

    if (m_fbo == 0)
    {
        glGenFramebuffers(1, &m_fbo);
        glGenRenderbuffers(1, &m_fbo_rbo);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, m_fbo_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, w, h);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_fbo_rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_fbo_width  = w;
    m_fbo_height = h;
}

bool GlFrameCapture::capture(int framebuffer_width, int framebuffer_height)
{
    if (framebuffer_width <= 0 || framebuffer_height <= 0)
        return false;

    Frame* slot = m_frames.reserve();
    if (!slot)
        return false;

    int read_w = framebuffer_width;
    int read_h = framebuffer_height;

    if (m_target_width > 0 && m_target_height > 0 &&
        (m_target_width != framebuffer_width || m_target_height != framebuffer_height))
    {
        ensure_blit_fbo(m_target_width, m_target_height);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
        glBlitFramebuffer(
            0,
            0,
            framebuffer_width,
            framebuffer_height,
            0,
            0,
            m_target_width,
            m_target_height,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        read_w = m_target_width;
        read_h = m_target_height;
    }

    slot->width       = read_w;
    slot->height      = read_h;
    slot->format      = PixelFormat::Rgb8;
    slot->compression = Compression::None;
    slot->pixels.resize(static_cast<size_t>(read_w) * static_cast<size_t>(read_h) * 3u);

    glReadPixels(0, 0, read_w, read_h, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // glReadPixels returns rows bottom-up; flip to top-down for encoder/display.
    const size_t stride = static_cast<size_t>(read_w) * 3u;
    for (int row = 0; row < read_h / 2; ++row)
    {
        auto beg = slot->pixels.begin();
        std::swap_ranges(
            beg + static_cast<ptrdiff_t>(row) * static_cast<ptrdiff_t>(stride),
            beg + static_cast<ptrdiff_t>(row + 1) * static_cast<ptrdiff_t>(stride),
            beg + static_cast<ptrdiff_t>(read_h - 1 - row) * static_cast<ptrdiff_t>(stride));
    }

    glBindTexture(GL_TEXTURE_2D, m_preview_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, read_w, read_h, 0, GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data());
    m_preview_ready  = true;
    m_preview_width  = read_w;
    m_preview_height = read_h;

    if (m_encode_fn && !m_encode_fn(*slot))
        return false;

    m_frames.deliver();
    if (m_after_deliver)
        m_after_deliver();

    return true;
}

void GlFrameCapture::set_after_deliver(std::function<void()> callback)
{
    m_after_deliver = std::move(callback);
}

void GlFrameCapture::set_encode_fn(std::function<bool(Frame&)> fn)
{
    m_encode_fn = std::move(fn);
}

void GlFrameCapture::set_target_size(int width, int height)
{
    m_target_width  = width;
    m_target_height = height;
}

uint32_t GlFrameCapture::preview_texture() const
{
    return m_preview_texture;
}
int GlFrameCapture::preview_width() const
{
    return m_preview_width;
}
int GlFrameCapture::preview_height() const
{
    return m_preview_height;
}
bool GlFrameCapture::preview_ready() const
{
    return m_preview_ready;
}

} // namespace stream
