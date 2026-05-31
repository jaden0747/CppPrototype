// ---------------------------------------------------------------------------
// udp_client.cpp — Phase 3: UDP shard receiver + FEC reassembly + H.264 decode
//
// Listens for UDP shards from a udp_server.  Reassembles frames via
// Reed-Solomon FEC recovery (tolerates ~25% packet loss by default) and
// decodes H.264 NALs to RGB for ImGui display.
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
#include "stream/tcp_socket.hpp"
#include "stream/udp_client_endpoint.hpp"
#include "stream/udp_packet.hpp"

#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstdint>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

struct UdpClientSettings
{
    int         listenPort = 9998;
    std::string logLevel   = "info";
    stream::GlAppConfig
        window{"UDP Client (Phase 3)", 1280, 720, true, true, {0.08f, 0.08f, 0.10f, 1.0f}};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(UdpClientSettings, listenPort, logLevel, window)
};

static SettingsItem<UdpClientSettings> g_settings("UdpClientSettings");

static dc::Mempool<stream::Frame>    g_framePool(3);
static dc::SenderPort<stream::Frame> g_frameSender;

static dc::Mempool<stream::ClientStats>    g_statsPool(2);
static dc::SenderPort<stream::ClientStats> g_statsSender;

namespace
{

void draw_stream_texture(const stream::GlFrameTexture& tex)
{
    ImVec2      avail  = ImGui::GetContentRegionAvail();
    const float aspect = tex.height() > 0
                             ? static_cast<float>(tex.width()) / static_cast<float>(tex.height())
                             : 16.0f / 9.0f;
    float dw = avail.x;
    float dh = dw / aspect;
    if (dh > avail.y)
    {
        dh = avail.y;
        dw = dh * aspect;
    }
    ImGui::SetCursorPos(
        ImVec2((avail.x - dw) * 0.5f + ImGui::GetCursorPosX(),
               (avail.y - dh) * 0.5f + ImGui::GetCursorPosY()));
    ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(tex.texture())), ImVec2(dw, dh));
}

// ---- Layer -----------------------------------------------------------------

class UdpClientLayer : public stream::GlLayer
{
public:
    UdpClientLayer(
        stream::GlFrameTexture&                texture,
        dc::ReceiverPort<stream::Frame>&       frameRx,
        dc::ReceiverPort<stream::ClientStats>& statsRx)
        : m_texture(texture)
        , m_frameRx(frameRx)
        , m_statsRx(statsRx)
    {
    }

    void onUpdate(float /*dt*/) override
    {
        m_texture.update(m_frameRx);

        m_statsRx.update();
        if (m_statsRx.hasNewData())
            if (const auto* s = m_statsRx.getData())
                m_stats = *s;
        m_statsRx.cleanup();

        SettingsRegistry::instance().tickAutoSave();
    }

    void onImGui() override
    {
        ImGui::Begin("UDP Client Status");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 3 — UDP / FEC Reassembly");
        ImGui::Separator();
        ImGui::Text("Listen port  : %d", g_settings->listenPort);
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

        ImGui::Separator();
        ImGui::TextDisabled("Codec: H.264 (libavcodec)");
        ImGui::TextDisabled("Transport: UDP, Reed-Solomon FEC recovery");
        ImGui::TextDisabled("MTU: %d bytes, payload: %d bytes/shard",
                            stream::UDP_MTU, stream::UDP_PAYLOAD_SIZE);
        ImGui::End();

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Video Stream (UDP decoded)");
        ImGui::PopStyleVar();

        if (m_texture.ready())
            draw_stream_texture(m_texture);
        else
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 120.0f, avail.y * 0.5f));
            ImGui::TextDisabled("No stream — start a udp_server and point it here");
        }
        ImGui::End();
    }

private:
    stream::GlFrameTexture&                m_texture;
    dc::ReceiverPort<stream::Frame>&       m_frameRx;
    dc::ReceiverPort<stream::ClientStats>& m_statsRx;
    stream::ClientStats                    m_stats;
};

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("udp_client_settings.json");

    Log::init(g_settings->logLevel, "udp_client.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("client");
    log->info("UDP client starting (listenPort={}, logLevel={})",
              g_settings->listenPort, g_settings->logLevel);

    stream::GlApp app(g_settings->window);
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::Frame>       frameRx;
    dc::ReceiverPort<stream::ClientStats> statsRx;
    frameRx.connect(g_frameSender);
    statsRx.connect(g_statsSender);

    stream::UdpClientEndpoint endpoint(g_frameSender, g_statsSender);

    stream::UdpClientConfig udpCfg;
    udpCfg.listen_port = static_cast<uint16_t>(g_settings->listenPort);
    endpoint.start(udpCfg);

    auto streamTexture = std::make_unique<stream::GlFrameTexture>();

    auto decoder = std::make_unique<stream::H264Decoder>();
    if (!decoder->init())
    {
        log->error("H264 decoder init failed");
        return 1;
    }
    streamTexture->set_decode_fn([&](const stream::Frame& h264, stream::Frame& rgb) {
        return decoder->decode(h264, rgb);
    });

    app.pushLayer(std::make_unique<UdpClientLayer>(*streamTexture, frameRx, statsRx));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    endpoint.stop();
    streamTexture.reset();

    SettingsRegistry::instance().saveJson("udp_client_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
