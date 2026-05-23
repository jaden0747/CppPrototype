#pragma once

#include "stream/frame.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace stream
{

struct FrameHeader
{
    uint32_t    width        = 0;
    uint32_t    height       = 0;
    PixelFormat format       = PixelFormat::Rgb8;
    Compression compression  = Compression::None;
    uint32_t    payload_size = 0;
};

inline constexpr size_t frame_header_wire_size = 16;

std::array<uint8_t, frame_header_wire_size> encode_header(const FrameHeader& header);
std::optional<FrameHeader>                  decode_header(const uint8_t* bytes, size_t size);
FrameHeader                                 header_from_frame(const Frame& frame);
bool                                        validate_header(const FrameHeader& header);

} // namespace stream
