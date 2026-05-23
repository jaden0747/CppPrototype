// ---------------------------------------------------------------------------
// stream_client.cpp - raw RGB frame streaming client
//
// App responsibilities:
//   - settings, logging, ImGui panels, and data-port wiring
// Stream library responsibilities:
//   - frame model/protocol, TCP endpoint, GL texture upload
// ---------------------------------------------------------------------------
#include "stream/stream_client.hpp"
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/gl_app.hpp"
#include "stream/gl_frame_texture.hpp"
#include "stream/tcp_socket.hpp"

#include <nlohmann/json.hpp>

#include <imgui.h>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

struct ClientSettings
{
    std::string serverIp   = "127.0.0.1";
    int         serverPort = 9999;
    std::string logLevel   = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ClientSettings, serverIp, serverPort, logLevel)
};

static SettingsItem<ClientSettings> g_settings("StreamClientSettings");

static dc::Mempool<stream::Frame>    g_framePool(3);
static dc::SenderPort<stream::Frame> g_frameSender;

static dc::Mempool<stream::ClientStats>    g_statsPool(2);
static dc::SenderPort<stream::ClientStats> g_statsSender;

namespace
{

void draw_stream_texture(const stream::GlFrameTexture& texture)
{
    ImVec2      avail  = ImGui::GetContentRegionAvail();
    const float aspect = texture.height() > 0
                             ? static_cast<float>(texture.width()) / static_cast<float>(texture.height())
                             : 16.0f / 9.0f;
    float       drawW  = avail.x;
    float       drawH  = drawW / aspect;
    if (drawH > avail.y)
    {
        drawH = avail.y;
        drawW = drawH * aspect;
    }
    ImGui::SetCursorPos(
        ImVec2((avail.x - drawW) * 0.5f + ImGui::GetCursorPosX(), (avail.y - drawH) * 0.5f + ImGui::GetCursorPosY()));
    ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(texture.texture())), ImVec2(drawW, drawH));
}

} // namespace

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("stream_client_settings.json");

    Log::init(g_settings->logLevel, "stream_client.log");
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("client");
    log->info(
        "Stream client starting (defaultServer={}:{}, logLevel={})",
        g_settings->serverIp,
        g_settings->serverPort,
        g_settings->logLevel);

    stream::GlApp app(
        stream::GlAppConfig{
            "Stream Client",
            1280,
            720,
            true,
            true,
            {0.08f, 0.08f, 0.10f, 1.0f},
        });
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::Frame>       frameReceiver;
    dc::ReceiverPort<stream::ClientStats> statsReceiver;
    frameReceiver.connect(g_frameSender);
    statsReceiver.connect(g_statsSender);

    stream::StreamClientEndpoint endpoint(g_frameSender, g_statsSender);
    endpoint.start();

    auto streamTexture = std::make_unique<stream::GlFrameTexture>();

    char ipBuf[256];
    std::strncpy(ipBuf, g_settings->serverIp.c_str(), sizeof(ipBuf) - 1);
    ipBuf[sizeof(ipBuf) - 1] = '\0';
    int portBuf              = g_settings->serverPort;

    stream::ClientStats stats;
    bool                connectRequested = false;

    app.run(
        stream::GlAppCallbacks{
            [&]
            {
                streamTexture->update(frameReceiver);

                statsReceiver.update();
                if (statsReceiver.hasNewData())
                {
                    if (const auto* s = statsReceiver.getData())
                    {
                        stats = *s;
                        if (!stats.connected &&
                            (stats.status == "Disconnected" || stats.status == "Connection refused"))
                        {
                            connectRequested = false;
                        }
                    }
                }
                statsReceiver.cleanup();
            },
            [&]
            {
                ImGui::Begin("Connection");

                bool isConnecting = connectRequested && !stats.connected;
                ImGui::InputText(
                    "Server IP",
                    ipBuf,
                    sizeof(ipBuf),
                    stats.connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);
                ImGui::InputInt("Port", &portBuf, stats.connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);

                if (!stats.connected && !isConnecting)
                {
                    if (ImGui::Button("Connect"))
                    {
                        stream::ClientEndpointConfig config;
                        config.server_ip   = ipBuf;
                        config.server_port = static_cast<uint16_t>(portBuf);
                        log->info("Requesting connect to {}:{}", config.server_ip, config.server_port);
                        endpoint.connect(config);
                        connectRequested = true;
                    }
                }
                else if (isConnecting)
                {
                    ImGui::BeginDisabled();
                    ImGui::Button("Connecting...");
                    ImGui::EndDisabled();
                }
                else
                {
                    if (ImGui::Button("Disconnect"))
                    {
                        log->info("Disconnect requested");
                        endpoint.disconnect();
                        connectRequested = false;
                    }
                }

                ImGui::Separator();
                if (stats.connected)
                    ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: %s", stats.status.c_str());
                else
                    ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.1f, 1.0f), "Status: %s", stats.status.c_str());

                ImGui::Text("FPS recv   : %.1f", stats.fps);
                ImGui::Text("Frames     : %llu", static_cast<unsigned long long>(stats.frames_received));
                ImGui::Text("Data recv  : %.2f MB", static_cast<double>(stats.bytes_received) / 1e6);
                if (streamTexture->ready())
                    ImGui::Text("Resolution : %d x %d", streamTexture->width(), streamTexture->height());
                ImGui::Text("Log level  : %s", g_settings->logLevel.c_str());
                ImGui::End();

                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
                ImGui::Begin("Video Stream");
                ImGui::PopStyleVar();

                if (streamTexture->ready())
                {
                    draw_stream_texture(*streamTexture);
                }
                else
                {
                    ImVec2 avail = ImGui::GetContentRegionAvail();
                    ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 100.0f, avail.y * 0.5f));
                    ImGui::TextDisabled("No stream - connect to a server");
                }
                ImGui::End();

                imguiSink->draw("Log");
            },
            {},
        });

    log->info("Shutting down");
    endpoint.stop();
    streamTexture.reset();

    g_settings->serverIp   = ipBuf;
    g_settings->serverPort = portBuf;
    SettingsRegistry::instance().saveJson("stream_client_settings.json");
    log->info("Settings saved to stream_client_settings.json. Exiting.");

    return 0;
}
