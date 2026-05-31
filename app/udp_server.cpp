// ---------------------------------------------------------------------------
// udp_server.cpp — Phase 3: H.264-encoded GL capture + UDP/FEC sender
//
// Same capture/encode pipeline as encode_server, but frames are transmitted
// over UDP via Reed-Solomon FEC sharding instead of a raw TCP stream.
// ---------------------------------------------------------------------------
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/ffmpeg_log.hpp"
#include "stream/gl_app.hpp"
#include "stream/gl_frame_capture.hpp"
#include "stream/h264_encoder.hpp"
#include "stream/imgui_log_layer.hpp"
#include "stream/tcp_socket.hpp"
#include "stream/udp_server_endpoint.hpp"
#include "stream/udp_packet.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <memory>
#include <string>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

struct UdpServerSettings
{
    std::string destIp   = "127.0.0.1";
    int         destPort = 9998;
    float       targetFps = 30.0f;
    int         bitrate   = 4'000'000;
    float       fecRatio  = 0.25f;
    std::string logLevel  = "info";
    stream::GlAppConfig
        window{"UDP Server (Phase 3)", 1280, 720, false, true, {0.10f, 0.10f, 0.14f, 1.0f}, "", 16.0f, 120.0f};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        UdpServerSettings, destIp, destPort, targetFps, bitrate, fecRatio, logLevel, window)
};

static SettingsItem<UdpServerSettings> g_settings("UdpServerSettings");

static dc::Mempool<stream::Frame>      g_framePool(3);
static dc::SenderPort<stream::Frame>   g_frameSender;
static dc::ReceiverPort<stream::Frame> g_frameReceiver;

static dc::Mempool<stream::ServerStats>    g_statsPool(2);
static dc::SenderPort<stream::ServerStats> g_statsSender;

namespace
{

float sanitize_fps(float fps)
{
    return std::max(1.0f, std::min(fps, 240.0f));
}

void draw_texture(uint32_t tex, int w, int h)
{
    ImVec2      avail  = ImGui::GetContentRegionAvail();
    const float aspect = h > 0 ? static_cast<float>(w) / static_cast<float>(h) : 16.0f / 9.0f;
    float       dw     = avail.x;
    float       dh     = dw / aspect;
    if (dh > avail.y)
    {
        dh = avail.y;
        dw = dh * aspect;
    }
    ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(tex)), ImVec2(dw, dh));
}

// ---- Layer -----------------------------------------------------------------

class UdpServerLayer : public stream::GlLayer
{
public:
    UdpServerLayer(
        stream::UdpServerEndpoint&             endpoint,
        stream::GlFrameCapture&                capture,
        dc::ReceiverPort<stream::ServerStats>& statsRx)
        : m_endpoint(endpoint)
        , m_capture(capture)
        , m_statsRx(statsRx)
    {
    }

    void onUpdate(float dt) override
    {
        m_statsRx.update();
        if (m_statsRx.hasNewData())
            if (const auto* s = m_statsRx.getData())
                m_stats = *s;
        m_statsRx.cleanup();

        if (dt > 0.0f)
            m_renderFps = m_renderFps * 0.95f + (1.0f / dt) * 0.05f;

        SettingsRegistry::instance().tickAutoSave();
    }

    void onImGui() override
    {
        ImGui::Begin("UDP Server Control");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 3 — UDP / FEC Sharding");
        ImGui::Separator();
        ImGui::Text("Render FPS   : %.1f", m_renderFps);
        ImGui::Text("Stream FPS   : %.1f target", g_settings->targetFps);
        ImGui::Text("Bitrate      : %.1f Mbps", g_settings->bitrate / 1e6);
        ImGui::Text("Destination  : %s:%d", g_settings->destIp.c_str(), g_settings->destPort);
        ImGui::Text("FEC ratio    : %.0f%%", g_settings->fecRatio * 100.0f);
        ImGui::Separator();

        if (m_stats.connected)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Sending to: %s", m_stats.peer.c_str());
            ImGui::Text("Frames sent  : %llu", static_cast<unsigned long long>(m_stats.frames_sent));
            ImGui::Text("Pkts sent    : %llu", static_cast<unsigned long long>(m_endpoint.packets_sent()));
            ImGui::Text("Data sent    : %.2f MB", static_cast<double>(m_stats.bytes_sent) / 1e6);

            if (m_stats.frames_sent > 0 && m_capture.preview_ready())
            {
                const double raw_mbs =
                    static_cast<double>(m_capture.preview_width()) *
                    static_cast<double>(m_capture.preview_height()) * 3.0 *
                    static_cast<double>(g_settings->targetFps) / 1e6;
                const double enc_mbs = g_settings->bitrate / 8.0 / 1e6;
                ImGui::Text("Bandwidth    : ~%.1f MB/s (FEC +%.0f%%)", enc_mbs * (1.0 + g_settings->fecRatio),
                            g_settings->fecRatio * 100.0f);
                ImGui::Text("vs raw       : %.1f MB/s (%.0fx smaller)", raw_mbs, raw_mbs / enc_mbs);
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.1f, 1.0f), "Waiting for first frame...");
        }

        ImGui::Separator();
        ImGui::TextDisabled("Codec: libx264, preset=ultrafast, tune=zerolatency");
        ImGui::TextDisabled("Transport: UDP, Reed-Solomon FEC (GF(2^8), Cauchy matrix)");
        ImGui::TextDisabled("MTU: %d bytes, payload: %d bytes/shard", stream::UDP_MTU, stream::UDP_PAYLOAD_SIZE);
        ImGui::End();

        ImGui::Begin("Captured Preview (raw)");
        ImGui::TextDisabled("Raw RGB before encoding:");
        if (m_capture.preview_ready())
            draw_texture(m_capture.preview_texture(), m_capture.preview_width(), m_capture.preview_height());
        else
            ImGui::Text("Waiting for first frame...");
        ImGui::End();
    }

    void onRender(int fbW, int fbH) override
    {
        const auto now = std::chrono::steady_clock::now();
        if (now < m_nextCapture)
            return;
        m_nextCapture = now + captureInterval();
        if (!m_capture.capture(fbW, fbH))
            Log::get("server")->warn("Capture/encode skipped");
    }

    std::chrono::steady_clock::time_point nextWakeup() const override { return m_nextCapture; }

private:
    std::chrono::nanoseconds captureInterval() const
    {
        const double secs = 1.0 / sanitize_fps(g_settings->targetFps);
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(secs));
    }

    stream::UdpServerEndpoint&             m_endpoint;
    stream::GlFrameCapture&                m_capture;
    dc::ReceiverPort<stream::ServerStats>& m_statsRx;

    stream::ServerStats                   m_stats;
    float                                 m_renderFps = 0.0f;
    std::chrono::steady_clock::time_point m_nextCapture = std::chrono::steady_clock::now();
};

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("udp_server_settings.json");

    Log::init(g_settings->logLevel, "udp_server.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("server");

    g_settings->targetFps = sanitize_fps(g_settings->targetFps);
    log->info("UDP server starting (dest={}:{}, fps={}, bitrate={}bps, fec={:.0f}%)",
              g_settings->destIp, g_settings->destPort,
              g_settings->targetFps, g_settings->bitrate,
              g_settings->fecRatio * 100.0f);

    stream::GlApp app(g_settings->window);
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_frameReceiver.connect(g_frameSender);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::ServerStats> statsRx;
    statsRx.connect(g_statsSender);

    stream::UdpServerConfig udpCfg;
    udpCfg.dest_ip   = g_settings->destIp;
    udpCfg.dest_port = static_cast<uint16_t>(g_settings->destPort);
    udpCfg.fec_ratio = g_settings->fecRatio;

    stream::UdpServerEndpoint endpoint(udpCfg, g_frameReceiver, g_statsSender);
    endpoint.start();

    auto capture = std::make_unique<stream::GlFrameCapture>(g_frameSender);
    capture->set_after_deliver([&endpoint] { endpoint.notify_frame_available(); });

    int logicalW = 0, logicalH = 0;
    glfwGetWindowSize(app.window(), &logicalW, &logicalH);
    capture->set_target_size(logicalW, logicalH);

    auto      encoder      = std::make_unique<stream::H264Encoder>();
    bool      encoderReady = false;
    const int targetBitrate = g_settings->bitrate;
    const int targetFpsInt  = static_cast<int>(sanitize_fps(g_settings->targetFps));

    capture->set_encode_fn([&](stream::Frame& frame) -> bool {
        if (!encoderReady)
        {
            stream::EncoderConfig cfg;
            cfg.fps     = targetFpsInt;
            cfg.bitrate = targetBitrate;
            if (!encoder->init(frame.width, frame.height, cfg))
                return false;
            encoderReady = true;
        }
        return encoder->encode(frame);
    });

    app.pushLayer(std::make_unique<UdpServerLayer>(endpoint, *capture, statsRx));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    endpoint.stop();
    capture.reset();

    SettingsRegistry::instance().saveJson("udp_server_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
