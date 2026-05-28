#include "stream/ffmpeg_log.hpp"

#include "mylib/log.hpp"

#include <cstring>
#include <mutex>

extern "C"
{
#include <libavutil/log.h>
}

namespace stream
{

namespace
{

void ffmpeg_log_cb(void* ptr, int level, const char* fmt, va_list vl)
{
    // Drop verbose/debug chatter; capture warnings and errors only.
    if (level > AV_LOG_WARNING)
        return;

    const spdlog::level::level_enum slevel = (level <= AV_LOG_ERROR) ? spdlog::level::err : spdlog::level::warn;

    // av_log_format_line2 prepends context info ([h264 @ ptr]) and formats the message.
    char buf[512];
    int  print_prefix = 1;
    av_log_format_line2(ptr, level, fmt, vl, buf, static_cast<int>(sizeof(buf)), &print_prefix);

    // Strip the trailing newline FFmpeg always appends.
    size_t len = strlen(buf);
    while (len > 0 && (buf[len - 1] == '\n' || buf[len - 1] == '\r'))
        buf[--len] = '\0';

    if (len > 0)
        Log::get("ffmpeg")->log(slevel, "{}", buf);
}

} // namespace

void install_ffmpeg_log_bridge()
{
    static std::once_flag s_once;
    std::call_once(s_once, [] { av_log_set_callback(ffmpeg_log_cb); });
}

} // namespace stream
