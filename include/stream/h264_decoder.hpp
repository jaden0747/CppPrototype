#pragma once

#include "stream/frame.hpp"

#include <memory>

namespace stream
{

class H264Decoder
{
public:
    H264Decoder();
    ~H264Decoder();

    H264Decoder(const H264Decoder&)            = delete;
    H264Decoder& operator=(const H264Decoder&) = delete;

    bool init();

    // Decode NAL bytes in h264 into raw RGB pixels written to rgb_out.
    // Returns false if no frame was produced yet (e.g. keyframe not received).
    bool decode(const Frame& h264, Frame& rgb_out);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace stream
