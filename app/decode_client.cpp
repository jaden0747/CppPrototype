// ---------------------------------------------------------------------------
// decode_client.cpp — Phase 2: H.264-decoding stream client
//
// Same structure as stream_client but the texture hook decodes incoming H.264
// NAL bytes back to raw RGB before uploading to the GL texture.
// ---------------------------------------------------------------------------
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/ffmpeg_log.hpp"
#include "stream/gl_app.hpp"
#include "stream/gl_frame_texture.hpp"
#include "stream/h264_decoder.hpp"
#include "stream/imgui_log_layer.hpp"
#include "stream/stream_client.hpp"
#include "stream/tcp_socket.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <cstring>
#include <memory>
#include <string>

struct DecodeClientSettings
{
    std::string         serverIp   = "127.0.0.1";
    int                 serverPort = 9999;
    std::string         logLevel   = "info";
    stream::GlAppConfig window{"Decode Client (H.264)", 1280, 720, true, true, {0.08f, 0.08f, 0.10f, 1.0f}};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(DecodeClientSettings, serverIp, serverPort, logLevel, window)
};

static SettingsItem<DecodeClientSettings> g_settings("DecodeClientSettings");

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

// ---- Layer -----------------------------------------------------------------

class DecodeClientLayer : public stream::GlLayer
{
public:
    DecodeClientLayer(
        stream::StreamClientEndpoint&          endpoint,
        stream::GlFrameTexture&                texture,
        dc::ReceiverPort<stream::Frame>&       frameReceiver,
        dc::ReceiverPort<stream::ClientStats>& statsReceiver)
        : m_endpoint(endpoint)
        , m_texture(texture)
        , m_frameReceiver(frameReceiver)
        , m_statsReceiver(statsReceiver)
    {
        std::strncpy(m_ipBuf, g_settings->serverIp.c_str(), sizeof(m_ipBuf) - 1);
        m_ipBuf[sizeof(m_ipBuf) - 1] = '\0';
        m_portBuf                    = g_settings->serverPort;
    }

    void onUpdate(float /*dt*/) override
    {
        m_texture.update(m_frameReceiver);

        m_statsReceiver.update();
        if (m_statsReceiver.hasNewData())
        {
            if (const auto* s = m_statsReceiver.getData())
            {
                m_stats = *s;
                if (!m_stats.connected && (m_stats.status == "Disconnected" || m_stats.status == "Connection refused"))
                    m_connectRequested = false;
            }
        }
        m_statsReceiver.cleanup();

        SettingsRegistry::instance().tickAutoSave();
    }

    void onImGui() override
    {
        ImGui::Begin("Connection");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 2 — H.264 Decoding");
        ImGui::Separator();

        bool isConnecting = m_connectRequested && !m_stats.connected;
        ImGui::InputText(
            "Server IP",
            m_ipBuf,
            sizeof(m_ipBuf),
            m_stats.connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);
        ImGui::InputInt("Port", &m_portBuf, m_stats.connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);

        if (!m_stats.connected && !isConnecting)
        {
            if (ImGui::Button("Connect"))
            {
                stream::ClientEndpointConfig config;
                config.server_ip   = m_ipBuf;
                config.server_port = static_cast<uint16_t>(m_portBuf);
                Log::get("client")->info("Requesting connect to {}:{}", config.server_ip, config.server_port);
                m_endpoint.connect(config);
                m_connectRequested = true;

                g_settings->serverIp   = m_ipBuf;
                g_settings->serverPort = m_portBuf;
                SettingsRegistry::instance().markDirty();
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
                Log::get("client")->info("Disconnect requested");
                m_endpoint.disconnect();
                m_connectRequested = false;
            }
        }

        ImGui::Separator();
        if (m_stats.connected)
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: %s", m_stats.status.c_str());
        else
            ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.1f, 1.0f), "Status: %s", m_stats.status.c_str());

        ImGui::Text("FPS recv     : %.1f", m_stats.fps);
        ImGui::Text("Frames       : %llu", static_cast<unsigned long long>(m_stats.frames_received));
        ImGui::Text("Data recv    : %.2f MB", static_cast<double>(m_stats.bytes_received) / 1e6);
        if (m_texture.ready())
            ImGui::Text("Resolution   : %d x %d", m_texture.width(), m_texture.height());
        ImGui::TextDisabled("Codec: H.264 (libavcodec)");
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Video Stream (decoded)");
        ImGui::PopStyleVar();

        if (m_texture.ready())
            draw_stream_texture(m_texture);
        else
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 100.0f, avail.y * 0.5f));
            ImGui::TextDisabled("No stream - connect to an encode_server");
        }
        ImGui::End();
    }

    const char* ip() const
    {
        return m_ipBuf;
    }
    int port() const
    {
        return m_portBuf;
    }

private:
    stream::StreamClientEndpoint&          m_endpoint;
    stream::GlFrameTexture&                m_texture;
    dc::ReceiverPort<stream::Frame>&       m_frameReceiver;
    dc::ReceiverPort<stream::ClientStats>& m_statsReceiver;

    stream::ClientStats m_stats;
    char                m_ipBuf[256]       = {};
    int                 m_portBuf          = 9999;
    bool                m_connectRequested = false;
};

} // namespace

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("decode_client_settings.json");

    Log::init(g_settings->logLevel, "decode_client.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("client");
    log->info(
        "Decode client starting (defaultServer={}:{}, logLevel={})",
        g_settings->serverIp,
        g_settings->serverPort,
        g_settings->logLevel);

    stream::GlApp app(g_settings->window);
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

    auto decoder = std::make_unique<stream::H264Decoder>();
    if (!decoder->init())
    {
        log->error("H264 decoder init failed");
        return 1;
    }
    streamTexture->set_decode_fn([&](const stream::Frame& h264, stream::Frame& rgb)
                                 { return decoder->decode(h264, rgb); });

    auto* clientLayer = new DecodeClientLayer(endpoint, *streamTexture, frameReceiver, statsReceiver);
    app.pushLayer(std::unique_ptr<stream::GlLayer>(clientLayer));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    endpoint.stop();
    streamTexture.reset();

    g_settings->serverIp   = clientLayer->ip();
    g_settings->serverPort = clientLayer->port();
    SettingsRegistry::instance().saveJson("decode_client_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
