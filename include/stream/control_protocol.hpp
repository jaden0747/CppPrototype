#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

namespace stream
{

// ---------------------------------------------------------------------------
// Control channel protocol — RTSP-like session negotiation over TCP.
//
// Wire format: [4-byte big-endian JSON length][JSON payload]
//
// The client sends request messages; the server replies with response messages.
// This is a strict request-response protocol (no server-initiated messages).
// ---------------------------------------------------------------------------

enum class ControlMethod : uint8_t
{
    Describe = 1,
    Setup    = 2,
    Play     = 3,
    Teardown = 4,
};

inline const char* to_string(ControlMethod m)
{
    switch (m)
    {
    case ControlMethod::Describe: return "DESCRIBE";
    case ControlMethod::Setup: return "SETUP";
    case ControlMethod::Play: return "PLAY";
    case ControlMethod::Teardown: return "TEARDOWN";
    }
    return "UNKNOWN";
}

// ---------------------------------------------------------------------------
// Session configuration — negotiated during SETUP
// ---------------------------------------------------------------------------

struct SessionConfig
{
    int         width   = 1280;
    int         height  = 720;
    int         fps     = 30;
    int         bitrate = 4'000'000;
    std::string codec   = "h264";
    float       fec_ratio = 0.25f;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(SessionConfig, width, height, fps, bitrate, codec, fec_ratio)
};

// ---------------------------------------------------------------------------
// Server capabilities — returned in DESCRIBE response
// ---------------------------------------------------------------------------

struct ServerCapabilities
{
    std::vector<std::string> codecs    = {"h264"};
    int                      max_width  = 1920;
    int                      max_height = 1080;
    int                      max_fps    = 60;
    uint16_t                 video_port = 9998;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServerCapabilities, codecs, max_width, max_height, max_fps, video_port)
};

// ---------------------------------------------------------------------------
// Request / Response messages (JSON-serialized)
// ---------------------------------------------------------------------------

struct ControlRequest
{
    std::string    method;
    nlohmann::json body = nlohmann::json::object();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ControlRequest, method, body)
};

struct ControlResponse
{
    int            status = 200;
    nlohmann::json body   = nlohmann::json::object();

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ControlResponse, status, body)
};

} // namespace stream
