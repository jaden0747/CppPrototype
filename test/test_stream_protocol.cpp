#include "stream/frame_protocol.hpp"

#include <gtest/gtest.h>

TEST(StreamFrame, ValidRgbFrame)
{
    stream::Frame frame;
    frame.width  = 4;
    frame.height = 3;
    frame.pixels.resize(4 * 3 * 3);

    EXPECT_EQ(frame.bytes_per_pixel(), 3);
    EXPECT_EQ(frame.expected_size(), 36u);
    EXPECT_TRUE(frame.valid());
}

TEST(StreamFrame, RejectsPayloadMismatch)
{
    stream::Frame frame;
    frame.width  = 4;
    frame.height = 3;
    frame.pixels.resize(10);

    EXPECT_FALSE(frame.valid());
}

TEST(StreamProtocol, HeaderRoundTrip)
{
    stream::FrameHeader header;
    header.width        = 640;
    header.height       = 480;
    header.format       = stream::PixelFormat::Rgb8;
    header.compression  = stream::Compression::None;
    header.payload_size = 640 * 480 * 3;

    const auto encoded = stream::encode_header(header);
    const auto decoded = stream::decode_header(encoded.data(), encoded.size());

    ASSERT_TRUE(decoded.has_value());
    EXPECT_EQ(decoded->width, header.width);
    EXPECT_EQ(decoded->height, header.height);
    EXPECT_EQ(decoded->format, header.format);
    EXPECT_EQ(decoded->compression, header.compression);
    EXPECT_EQ(decoded->payload_size, header.payload_size);
}

TEST(StreamProtocol, RejectsInvalidDimensions)
{
    stream::FrameHeader header;
    header.width        = 0;
    header.height       = 480;
    header.payload_size = 100;

    EXPECT_FALSE(stream::validate_header(header));
}

TEST(StreamProtocol, RejectsOversizedPayload)
{
    stream::FrameHeader header;
    header.width        = 8192;
    header.height       = 8192;
    header.format       = stream::PixelFormat::Rgb8;
    header.compression  = stream::Compression::None;
    header.payload_size = static_cast<uint32_t>(stream::max_frame_payload_size + 1u);

    EXPECT_FALSE(stream::validate_header(header));
}

TEST(StreamProtocol, AcceptsFutureCompressedPayloadAsStub)
{
    stream::FrameHeader header;
    header.width        = 1920;
    header.height       = 1080;
    header.format       = stream::PixelFormat::Rgb8;
    header.compression  = stream::Compression::H264;
    header.payload_size = 4096;

    EXPECT_TRUE(stream::validate_header(header));
}
