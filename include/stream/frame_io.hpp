#pragma once

#include "stream/frame_protocol.hpp"
#include "stream/tcp_socket.hpp"

#include <optional>

namespace stream
{

bool                       write_frame(TcpConnection& conn, const Frame& frame);
std::optional<FrameHeader> read_frame_header(TcpConnection& conn);
bool                       read_frame_payload(TcpConnection& conn, const FrameHeader& header, Frame& out);
bool                       read_frame(TcpConnection& conn, Frame& out);
bool                       drain_frame_payload(TcpConnection& conn, const FrameHeader& header);

} // namespace stream
