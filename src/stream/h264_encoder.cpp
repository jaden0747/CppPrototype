#include "stream/h264_encoder.hpp"

#include "mylib/log.hpp"

#include <algorithm>
#include <cstring>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}

namespace stream
{

struct H264Encoder::Impl
{
    AVCodecContext* ctx          = nullptr;
    AVFrame*        yuv          = nullptr;
    AVPacket*       pkt          = nullptr;
    SwsContext*     sws          = nullptr; // RGB(input) → YUV420P(encode), handles rescale too
    int64_t         pts          = 0;
    int             input_width  = 0; // captured frame size (may be 2× on retina)
    int             input_height = 0;
    int             enc_width    = 0; // encoded video size (logical window size)
    int             enc_height   = 0;
};

H264Encoder::H264Encoder()
    : m_impl(std::make_unique<Impl>())
{
}

H264Encoder::~H264Encoder()
{
    if (m_impl->sws)
        sws_freeContext(m_impl->sws);
    if (m_impl->yuv)
        av_frame_free(&m_impl->yuv);
    if (m_impl->pkt)
        av_packet_free(&m_impl->pkt);
    if (m_impl->ctx)
        avcodec_free_context(&m_impl->ctx);
}

bool H264Encoder::init(int input_width, int input_height, EncoderConfig config)
{
    auto log = Log::get("h264enc");

    // Resolve encode resolution: explicit config wins, else use capture size.
    // Force even dimensions (H.264 requirement).
    int enc_w = (config.encode_width > 0 ? config.encode_width : input_width) & ~1;
    int enc_h = (config.encode_height > 0 ? config.encode_height : input_height) & ~1;

    const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    if (!codec)
    {
        log->error("libx264 not found — rebuild FFmpeg with --enable-libx264");
        return false;
    }

    m_impl->ctx         = avcodec_alloc_context3(codec);
    AVCodecContext* ctx = m_impl->ctx;
    ctx->width          = enc_w;
    ctx->height         = enc_h;
    ctx->time_base      = {1, config.fps};
    ctx->framerate      = {config.fps, 1};
    ctx->bit_rate       = config.bitrate;
    ctx->pix_fmt        = AV_PIX_FMT_YUV420P;
    // Short GOP so mid-stream clients resync quickly (at most ~0.5 s wait for IDR).
    ctx->gop_size     = std::max(1, config.fps / 2);
    ctx->max_b_frames = 0;

    av_opt_set(ctx->priv_data, "preset", "ultrafast", 0);
    av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
    // repeat_headers=1: SPS+PPS before every IDR so mid-stream clients can decode
    // without out-of-band extradata. No slice-max-size: that's a UDP/MTU hint and
    // causes 40-70 slices/frame on TCP, exceeding the decoder's MAX_SLICES=32.
    av_opt_set(ctx->priv_data, "x264-params", "repeat_headers=1", 0);

    if (avcodec_open2(ctx, codec, nullptr) < 0)
    {
        log->error("avcodec_open2 failed");
        avcodec_free_context(&m_impl->ctx);
        return false;
    }

    m_impl->yuv         = av_frame_alloc();
    m_impl->yuv->format = AV_PIX_FMT_YUV420P;
    m_impl->yuv->width  = enc_w;
    m_impl->yuv->height = enc_h;
    av_frame_get_buffer(m_impl->yuv, 32);

    m_impl->pkt = av_packet_alloc();

    // Single sws pass: RGB(input_w×input_h) → YUV420P(enc_w×enc_h)
    // Handles both colorconv and any downscale needed for retina displays.
    const int sws_flags = (input_width != enc_w || input_height != enc_h)
                              ? SWS_BILINEAR       // higher quality for rescale
                              : SWS_FAST_BILINEAR; // fast path when no rescale
    m_impl->sws         = sws_getContext(
        input_width,
        input_height,
        AV_PIX_FMT_RGB24,
        enc_w,
        enc_h,
        AV_PIX_FMT_YUV420P,
        sws_flags,
        nullptr,
        nullptr,
        nullptr);

    m_impl->input_width  = input_width;
    m_impl->input_height = input_height;
    m_impl->enc_width    = enc_w;
    m_impl->enc_height   = enc_h;

    if (input_width != enc_w || input_height != enc_h)
        log->info(
            "H264 encoder ready (capture {}x{} → encode {}x{} @{}fps {:.1f}Mbps)",
            input_width,
            input_height,
            enc_w,
            enc_h,
            config.fps,
            config.bitrate / 1'000'000.0);
    else
        log->info(
            "H264 encoder ready ({}x{} @{}fps {:.1f}Mbps)", enc_w, enc_h, config.fps, config.bitrate / 1'000'000.0);
    return true;
}

bool H264Encoder::encode(Frame& frame)
{
    if (!m_impl->ctx)
        return false;
    if (frame.compression != Compression::None || frame.format != PixelFormat::Rgb8)
        return false;
    if (frame.width != m_impl->input_width || frame.height != m_impl->input_height)
        return false;
    if (frame.pixels.empty())
        return false;

    av_frame_make_writable(m_impl->yuv);

    // sws handles both colorconv and any rescale in one pass
    const uint8_t* src_data[1]   = {frame.pixels.data()};
    int            src_stride[1] = {m_impl->input_width * 3};
    sws_scale(m_impl->sws, src_data, src_stride, 0, m_impl->input_height, m_impl->yuv->data, m_impl->yuv->linesize);
    m_impl->yuv->pts = m_impl->pts++;

    if (avcodec_send_frame(m_impl->ctx, m_impl->yuv) < 0)
        return false;

    frame.pixels.clear();
    while (avcodec_receive_packet(m_impl->ctx, m_impl->pkt) == 0)
    {
        const size_t off = frame.pixels.size();
        frame.pixels.resize(off + static_cast<size_t>(m_impl->pkt->size));
        std::memcpy(frame.pixels.data() + off, m_impl->pkt->data, static_cast<size_t>(m_impl->pkt->size));
        av_packet_unref(m_impl->pkt);
    }

    if (frame.pixels.empty())
        return false; // zerolatency should prevent this, but guard anyway

    frame.compression = Compression::H264;
    // Update frame dimensions to reflect encoded (possibly rescaled) size.
    // The decoder will reconstruct at these dimensions.
    frame.width  = m_impl->enc_width;
    frame.height = m_impl->enc_height;
    return true;
}

} // namespace stream
