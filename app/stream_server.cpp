// ---------------------------------------------------------------------------
// stream_server.cpp - raw RGB frame streaming server
//
// App responsibilities:
//   - settings, logging, ImGui panels, and data-port wiring
// Stream library responsibilities:
//   - frame model/protocol, TCP endpoint, GL framebuffer capture
// ---------------------------------------------------------------------------
#include "stream/stream_server.hpp"
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/gl_app.hpp"
#include "stream/gl_frame_capture.hpp"
#include "stream/tcp_socket.hpp"

#include <nlohmann/json.hpp>

#include <imgui.h>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

struct ServerSettings
{
    int         port     = 9999;
    std::string logLevel = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServerSettings, port, logLevel)
};

static SettingsItem<ServerSettings> g_settings("StreamServerSettings");

static dc::Mempool<stream::Frame>      g_framePool(3);
static dc::SenderPort<stream::Frame>   g_frameSender;
static dc::ReceiverPort<stream::Frame> g_frameReceiver;

static dc::Mempool<stream::ServerStats>    g_statsPool(2);
static dc::SenderPort<stream::ServerStats> g_statsSender;

namespace
{

void draw_texture(uint32_t texture, int width, int height)
{
    ImVec2      avail  = ImGui::GetContentRegionAvail();
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 16.0f / 9.0f;
    float       drawW  = avail.x;
    float       drawH  = drawW / aspect;
    if (drawH > avail.y)
    {
        drawH = avail.y;
        drawW = drawH * aspect;
    }
    ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(texture)), ImVec2(drawW, drawH));
}

} // namespace

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("stream_server_settings.json");

    Log::init(g_settings->logLevel, "stream_server.log");
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("server");
    log->info("Stream server starting (port={}, logLevel={})", g_settings->port, g_settings->logLevel);

    stream::GlApp app(
        stream::GlAppConfig{
            "Stream Server",
            1280,
            720,
            true,
            true,
            {0.10f, 0.10f, 0.14f, 1.0f},
        });
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_frameReceiver.connect(g_frameSender);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::ServerStats> statsReceiver;
    statsReceiver.connect(g_statsSender);

    stream::StreamServerEndpoint endpoint(
        stream::ServerEndpointConfig{static_cast<uint16_t>(g_settings->port)}, g_frameReceiver, g_statsSender);
    endpoint.start();

    auto capture = std::make_unique<stream::GlFrameCapture>(g_frameSender);
    capture->set_after_deliver([&endpoint] { endpoint.notify_frame_available(); });

    using clock         = std::chrono::steady_clock;
    auto  lastFrameTime = clock::now();
    float renderFps     = 0.0f;

    stream::ServerStats stats;

    app.run(
        stream::GlAppCallbacks{
            [&]
            {
                statsReceiver.update();
                if (statsReceiver.hasNewData())
                {
                    if (const auto* s = statsReceiver.getData())
                        stats = *s;
                }
                statsReceiver.cleanup();

                auto  now = clock::now();
                float dt  = std::chrono::duration<float>(now - lastFrameTime).count();
                if (dt > 0.0f)
                    renderFps = renderFps * 0.95f + (1.0f / dt) * 0.05f;
                lastFrameTime = now;
            },
            [&]
            {
                ImGui::Begin("Server Control");
                ImGui::Text("Render FPS : %.1f", renderFps);
                ImGui::Text("Port       : %d", g_settings->port);
                ImGui::Text("Log level  : %s", g_settings->logLevel.c_str());
                ImGui::Separator();
                if (stats.connected)
                {
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Connected: %s", stats.peer.c_str());
                    ImGui::Text("Frames sent : %llu", static_cast<unsigned long long>(stats.frames_sent));
                    ImGui::Text("Data sent   : %.2f MB", static_cast<double>(stats.bytes_sent) / 1e6);
                }
                else
                {
                    ImGui::TextColored(
                        ImVec4(1.0f, 0.8f, 0.1f, 1.0f), "Waiting for client on :%d ...", g_settings->port);
                }
                ImGui::Separator();
                ImGui::TextDisabled("Protocol: [width][height][format][compression][len][payload]");
                ImGui::End();

                ImGui::Begin("Captured Preview");
                ImGui::TextDisabled("What the server is streaming right now:");
                if (capture->preview_ready())
                    draw_texture(capture->preview_texture(), capture->preview_width(), capture->preview_height());
                else
                    ImGui::Text("Waiting for first frame...");
                ImGui::End();

                imguiSink->draw("Log");
            },
            [&](int fbW, int fbH)
            {
                if (!capture->capture(fbW, fbH))
                    log->warn("Frame pool exhausted; skipping capture this frame");
            },
        });

    log->info("Shutting down");
    endpoint.stop();
    capture.reset();

    SettingsRegistry::instance().saveJson("stream_server_settings.json");
    log->info("Settings saved to stream_server_settings.json. Exiting.");

    return 0;
}
