#include "stream/h264_decoder.hpp"

#include "mylib/log.hpp"

#include <cstring>
#include <vector>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace stream
{

struct H264Decoder::Impl
{
    AVCodecContext* ctx   = nullptr;
    AVFrame*        frm   = nullptr;
    AVPacket*       pkt   = nullptr;
    SwsContext*     sws   = nullptr;
    int             sws_w = 0;
    int             sws_h = 0;
};

H264Decoder::H264Decoder()
    : m_impl(std::make_unique<Impl>())
{
}

H264Decoder::~H264Decoder()
{
    if (m_impl->sws)
        sws_freeContext(m_impl->sws);
    if (m_impl->frm)
        av_frame_free(&m_impl->frm);
    if (m_impl->pkt)
        av_packet_free(&m_impl->pkt);
    if (m_impl->ctx)
        avcodec_free_context(&m_impl->ctx);
}

bool H264Decoder::init()
{
    auto log = Log::get("h264dec");

    const AVCodec* codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec)
    {
        log->error("H264 decoder not found");
        return false;
    }

    m_impl->ctx = avcodec_alloc_context3(codec);
    if (avcodec_open2(m_impl->ctx, codec, nullptr) < 0)
    {
        log->error("avcodec_open2 failed");
        return false;
    }

    m_impl->frm = av_frame_alloc();
    m_impl->pkt = av_packet_alloc();

    log->info("H264 decoder ready");
    return true;
}

bool H264Decoder::decode(const Frame& h264, Frame& rgb_out)
{
    if (!m_impl->ctx)
        return false;
    if (h264.compression != Compression::H264 || h264.pixels.empty())
        return false;

    // Our framing protocol delivers exactly one encoder output per call, so
    // no parser is needed — feed the whole blob as a single packet.
    // FFmpeg requires AV_INPUT_BUFFER_PADDING_SIZE zero-bytes after the payload.
    std::vector<uint8_t> padded(h264.pixels.size() + AV_INPUT_BUFFER_PADDING_SIZE, 0u);
    std::memcpy(padded.data(), h264.pixels.data(), h264.pixels.size());

    av_packet_unref(m_impl->pkt);
    m_impl->pkt->data = padded.data();
    m_impl->pkt->size = static_cast<int>(h264.pixels.size());

    int ret           = avcodec_send_packet(m_impl->ctx, m_impl->pkt);
    m_impl->pkt->data = nullptr; // packet does not own this buffer
    m_impl->pkt->size = 0;
    if (ret < 0)
        return false;

    bool got_frame = false;
    while (avcodec_receive_frame(m_impl->ctx, m_impl->frm) == 0)
    {
        const int w = m_impl->frm->width;
        const int h = m_impl->frm->height;

        if (!m_impl->sws || m_impl->sws_w != w || m_impl->sws_h != h)
        {
            if (m_impl->sws)
                sws_freeContext(m_impl->sws);
            m_impl->sws = sws_getContext(
                w, h, AV_PIX_FMT_YUV420P, w, h, AV_PIX_FMT_RGB24, SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
            m_impl->sws_w = w;
            m_impl->sws_h = h;
        }

        rgb_out.width       = w;
        rgb_out.height      = h;
        rgb_out.format      = PixelFormat::Rgb8;
        rgb_out.compression = Compression::None;
        rgb_out.pixels.resize(static_cast<size_t>(w) * static_cast<size_t>(h) * 3u);

        uint8_t* dst_data[1]   = {rgb_out.pixels.data()};
        int      dst_stride[1] = {w * 3};
        sws_scale(
            m_impl->sws,
            const_cast<const uint8_t* const*>(m_impl->frm->data),
            m_impl->frm->linesize,
            0,
            h,
            dst_data,
            dst_stride);

        av_frame_unref(m_impl->frm);
        got_frame = true;
        break; // one decoded frame per NAL blob
    }

    return got_frame;
}

} // namespace stream
