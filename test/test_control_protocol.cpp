#include "stream/control_protocol.hpp"

#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

// ---------------------------------------------------------------------------
// ControlMethod to_string
// ---------------------------------------------------------------------------

TEST(ControlProtocol, MethodToString)
{
    EXPECT_STREQ(stream::to_string(stream::ControlMethod::Describe), "DESCRIBE");
    EXPECT_STREQ(stream::to_string(stream::ControlMethod::Setup), "SETUP");
    EXPECT_STREQ(stream::to_string(stream::ControlMethod::Play), "PLAY");
    EXPECT_STREQ(stream::to_string(stream::ControlMethod::Teardown), "TEARDOWN");
}

// ---------------------------------------------------------------------------
// SessionConfig serialization round-trip
// ---------------------------------------------------------------------------

TEST(ControlProtocol, SessionConfigRoundTrip)
{
    stream::SessionConfig cfg;
    cfg.width     = 1920;
    cfg.height    = 1080;
    cfg.fps       = 60;
    cfg.bitrate   = 8'000'000;
    cfg.codec     = "hevc";
    cfg.fec_ratio = 0.30f;

    nlohmann::json j;
    to_json(j, cfg);

    stream::SessionConfig decoded;
    from_json(j, decoded);

    EXPECT_EQ(decoded.width, 1920);
    EXPECT_EQ(decoded.height, 1080);
    EXPECT_EQ(decoded.fps, 60);
    EXPECT_EQ(decoded.bitrate, 8'000'000);
    EXPECT_EQ(decoded.codec, "hevc");
    EXPECT_FLOAT_EQ(decoded.fec_ratio, 0.30f);
}

TEST(ControlProtocol, SessionConfigDefaults)
{
    stream::SessionConfig cfg;
    EXPECT_EQ(cfg.width, 1280);
    EXPECT_EQ(cfg.height, 720);
    EXPECT_EQ(cfg.fps, 30);
    EXPECT_EQ(cfg.bitrate, 4'000'000);
    EXPECT_EQ(cfg.codec, "h264");
    EXPECT_FLOAT_EQ(cfg.fec_ratio, 0.25f);
}

// ---------------------------------------------------------------------------
// ServerCapabilities serialization round-trip
// ---------------------------------------------------------------------------

TEST(ControlProtocol, ServerCapabilitiesRoundTrip)
{
    stream::ServerCapabilities caps;
    caps.codecs     = {"h264", "hevc", "av1"};
    caps.max_width  = 3840;
    caps.max_height = 2160;
    caps.max_fps    = 120;
    caps.video_port = 5000;

    nlohmann::json j;
    to_json(j, caps);

    stream::ServerCapabilities decoded;
    from_json(j, decoded);

    EXPECT_EQ(decoded.codecs.size(), 3u);
    EXPECT_EQ(decoded.codecs[0], "h264");
    EXPECT_EQ(decoded.codecs[1], "hevc");
    EXPECT_EQ(decoded.codecs[2], "av1");
    EXPECT_EQ(decoded.max_width, 3840);
    EXPECT_EQ(decoded.max_height, 2160);
    EXPECT_EQ(decoded.max_fps, 120);
    EXPECT_EQ(decoded.video_port, 5000);
}

// ---------------------------------------------------------------------------
// ControlRequest / ControlResponse serialization
// ---------------------------------------------------------------------------

TEST(ControlProtocol, RequestRoundTrip)
{
    stream::ControlRequest req;
    req.method = "SETUP";
    req.body   = {{"width", 1280}, {"height", 720}};

    nlohmann::json j;
    to_json(j, req);

    stream::ControlRequest decoded;
    from_json(j, decoded);

    EXPECT_EQ(decoded.method, "SETUP");
    EXPECT_EQ(decoded.body["width"].get<int>(), 1280);
    EXPECT_EQ(decoded.body["height"].get<int>(), 720);
}

TEST(ControlProtocol, ResponseRoundTrip)
{
    stream::ControlResponse resp;
    resp.status = 404;
    resp.body   = {{"error", "Session not found"}};

    nlohmann::json j;
    to_json(j, resp);

    stream::ControlResponse decoded;
    from_json(j, decoded);

    EXPECT_EQ(decoded.status, 404);
    EXPECT_EQ(decoded.body["error"].get<std::string>(), "Session not found");
}

TEST(ControlProtocol, ResponseDefaults)
{
    stream::ControlResponse resp;
    EXPECT_EQ(resp.status, 200);
    EXPECT_TRUE(resp.body.is_object());
    EXPECT_TRUE(resp.body.empty());
}

// ---------------------------------------------------------------------------
// SessionConfig partial JSON (missing fields get defaults)
// ---------------------------------------------------------------------------

TEST(ControlProtocol, SessionConfigPartialJson)
{
    nlohmann::json j = {{"width", 640}, {"height", 480}};

    stream::SessionConfig cfg;
    from_json(j, cfg);

    EXPECT_EQ(cfg.width, 640);
    EXPECT_EQ(cfg.height, 480);
    // Defaults for unspecified fields
    EXPECT_EQ(cfg.fps, 30);
    EXPECT_EQ(cfg.bitrate, 4'000'000);
    EXPECT_EQ(cfg.codec, "h264");
}
