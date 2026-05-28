#pragma once

namespace stream
{

// Redirect FFmpeg's av_log output through spdlog (logger name: "ffmpeg").
// Only warnings and errors are forwarded; verbose/debug messages are dropped.
// Safe to call multiple times — installs the callback exactly once.
void install_ffmpeg_log_bridge();

} // namespace stream
