#include "stream/frame_protocol.hpp"

#include <algorithm>
#include <cstring>
#include <limits>

namespace stream
{
namespace
{

void put_u32(uint8_t* out, uint32_t value)
{
    out[0] = static_cast<uint8_t>((value >> 24) & 0xffu);
    out[1] = static_cast<uint8_t>((value >> 16) & 0xffu);
    out[2] = static_cast<uint8_t>((value >> 8) & 0xffu);
    out[3] = static_cast<uint8_t>(value & 0xffu);
}

uint32_t get_u32(const uint8_t* in)
{
    return (static_cast<uint32_t>(in[0]) << 24) | (static_cast<uint32_t>(in[1]) << 16) |
           (static_cast<uint32_t>(in[2]) << 8) | static_cast<uint32_t>(in[3]);
}

bool known_format(PixelFormat format)
{
    return format == PixelFormat::Rgb8;
}

bool known_compression(Compression compression)
{
    return compression == Compression::None || compression == Compression::H264;
}

} // namespace

int Frame::bytes_per_pixel() const
{
    if (compression != Compression::None)
        return 0;
    switch (format)
    {
    case PixelFormat::Rgb8:
        return 3;
    }
    return 0;
}

size_t Frame::expected_size() const
{
    const int bpp = bytes_per_pixel();
    if (width <= 0 || height <= 0 || bpp <= 0)
        return 0;

    const auto w = static_cast<size_t>(width);
    const auto h = static_cast<size_t>(height);
    const auto b = static_cast<size_t>(bpp);
    if (w > std::numeric_limits<size_t>::max() / h)
        return 0;
    const size_t pixels_count = w * h;
    if (pixels_count > std::numeric_limits<size_t>::max() / b)
        return 0;
    return pixels_count * b;
}

bool Frame::valid() const
{
    const size_t expected = expected_size();
    return expected > 0 && expected <= max_frame_payload_size && pixels.size() == expected;
}

std::array<uint8_t, frame_header_wire_size> encode_header(const FrameHeader& header)
{
    std::array<uint8_t, frame_header_wire_size> out{};
    put_u32(out.data() + 0, header.width);
    put_u32(out.data() + 4, header.height);
    out[8]  = static_cast<uint8_t>(header.format);
    out[9]  = static_cast<uint8_t>(header.compression);
    out[10] = 0;
    out[11] = 0;
    put_u32(out.data() + 12, header.payload_size);
    return out;
}

std::optional<FrameHeader> decode_header(const uint8_t* bytes, size_t size)
{
    if (!bytes || size != frame_header_wire_size)
        return std::nullopt;

    FrameHeader header;
    header.width        = get_u32(bytes + 0);
    header.height       = get_u32(bytes + 4);
    header.format       = static_cast<PixelFormat>(bytes[8]);
    header.compression  = static_cast<Compression>(bytes[9]);
    header.payload_size = get_u32(bytes + 12);

    if (!validate_header(header))
        return std::nullopt;
    return header;
}

FrameHeader header_from_frame(const Frame& frame)
{
    return FrameHeader{
        static_cast<uint32_t>(std::max(frame.width, 0)),
        static_cast<uint32_t>(std::max(frame.height, 0)),
        frame.format,
        frame.compression,
        static_cast<uint32_t>(frame.pixels.size()),
    };
}

bool validate_header(const FrameHeader& header)
{
    if (header.width == 0 || header.height == 0)
        return false;
    if (!known_format(header.format) || !known_compression(header.compression))
        return false;
    if (header.payload_size == 0 || header.payload_size > max_frame_payload_size)
        return false;

    if (header.compression == Compression::None)
    {
        Frame frame;
        frame.width       = static_cast<int>(header.width);
        frame.height      = static_cast<int>(header.height);
        frame.format      = header.format;
        frame.compression = header.compression;
        return frame.expected_size() == header.payload_size;
    }

    // Stub for future encoded formats. The endpoint can carry the payload, but
    // Phase 1 has no decoder/encoder for compressed video yet.
    return true;
}

} // namespace stream
