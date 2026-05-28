#pragma once

#include "stream/frame.hpp"

#include <memory>

namespace stream
{

struct EncoderConfig
{
    int fps     = 30;
    int bitrate = 4'000'000; // 4 Mbps

    // Target encode resolution. When non-zero and different from the captured
    // frame size, the encoder rescales in the same sws pass as the colorconv
    // (RGB→YUV420P). Useful for retina displays where glReadPixels returns 2×
    // the logical window dimensions but we want to encode at logical resolution.
    // Set to 0 to encode at the captured frame's native size.
    int encode_width  = 0;
    int encode_height = 0;
};

class H264Encoder
{
public:
    H264Encoder();
    ~H264Encoder();

    H264Encoder(const H264Encoder&)            = delete;
    H264Encoder& operator=(const H264Encoder&) = delete;

    bool init(int width, int height, EncoderConfig config = {});

    // In-place: raw RGB frame → H264 NAL bytes.
    // Returns false if the encoder produced no output (e.g. still buffering).
    bool encode(Frame& frame);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace stream
