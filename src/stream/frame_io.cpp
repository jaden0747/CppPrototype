#include "stream/frame_io.hpp"

#include <algorithm>
#include <array>
#include <vector>

namespace stream
{

bool write_frame(TcpConnection& conn, const Frame& frame)
{
    if (!frame.valid())
        return false;

    const FrameHeader header = header_from_frame(frame);
    if (!validate_header(header))
        return false;

    const auto bytes = encode_header(header);
    return conn.write_exact(bytes.data(), bytes.size()) && conn.write_exact(frame.pixels.data(), frame.pixels.size());
}

std::optional<FrameHeader> read_frame_header(TcpConnection& conn)
{
    std::array<uint8_t, frame_header_wire_size> bytes{};
    if (!conn.read_exact(bytes.data(), bytes.size()))
        return std::nullopt;
    return decode_header(bytes.data(), bytes.size());
}

bool read_frame_payload(TcpConnection& conn, const FrameHeader& header, Frame& out)
{
    if (!validate_header(header))
        return false;

    out.width       = static_cast<int>(header.width);
    out.height      = static_cast<int>(header.height);
    out.format      = header.format;
    out.compression = header.compression;
    out.pixels.resize(header.payload_size);
    return conn.read_exact(out.pixels.data(), out.pixels.size());
}

bool read_frame(TcpConnection& conn, Frame& out)
{
    const auto header = read_frame_header(conn);
    return header && read_frame_payload(conn, *header, out);
}

bool drain_frame_payload(TcpConnection& conn, const FrameHeader& header)
{
    if (!validate_header(header))
        return false;

    std::array<uint8_t, 16 * 1024> buffer{};
    size_t                         remaining = header.payload_size;
    while (remaining > 0)
    {
        const size_t chunk = std::min(remaining, buffer.size());
        if (!conn.read_exact(buffer.data(), chunk))
            return false;
        remaining -= chunk;
    }
    return true;
}

} // namespace stream
