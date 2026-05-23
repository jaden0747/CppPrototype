#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace stream
{

enum class PixelFormat : uint8_t
{
    Rgb8 = 1,
};

enum class Compression : uint8_t
{
    None = 0,
    H264 = 1,
};

struct Frame
{
    std::vector<uint8_t> pixels;
    int                  width       = 0;
    int                  height      = 0;
    PixelFormat          format      = PixelFormat::Rgb8;
    Compression          compression = Compression::None;

    int    bytes_per_pixel() const;
    size_t expected_size() const;
    bool   valid() const;
};

inline constexpr size_t max_frame_payload_size = 64u * 1024u * 1024u;

} // namespace stream
