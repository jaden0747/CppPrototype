// ---------------------------------------------------------------------------
// rtsp_server.cpp — Phase 4: RTSP-like control channel + UDP/FEC video stream
//
// The server waits for a client to negotiate via the control channel
// (DESCRIBE → SETUP → PLAY) before starting the UDP video stream.
// The stream parameters (resolution, bitrate, FEC ratio) are negotiated
// during SETUP, separating the control plane from the data plane.
// ---------------------------------------------------------------------------
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/control_protocol.hpp"
#include "stream/control_server.hpp"
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

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

struct RtspServerSettings
{
    int         controlPort = 48010;
    int         videoPort   = 9998;
    std::string destIp      = "127.0.0.1";
    float       targetFps   = 30.0f;
    int         bitrate     = 4'000'000;
    float       fecRatio    = 0.25f;
    std::string logLevel    = "info";
    stream::GlAppConfig
        window{"RTSP Server (Phase 4)", 1280, 720, false, true, {0.10f, 0.10f, 0.14f, 1.0f}, "", 16.0f, 120.0f};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        RtspServerSettings, controlPort, videoPort, destIp, targetFps, bitrate, fecRatio, logLevel, window)
};

static SettingsItem<RtspServerSettings> g_settings("RtspServerSettings");

static dc::Mempool<stream::Frame>      g_framePool(3);
static dc::SenderPort<stream::Frame>   g_frameSender;
static dc::ReceiverPort<stream::Frame> g_frameReceiver;

static dc::Mempool<stream::ServerStats>    g_statsPool(2);
static dc::SenderPort<stream::ServerStats> g_statsSender;

namespace
{

float sanitize_fps(float fps) { return std::max(1.0f, std::min(fps, 240.0f)); }

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

class RtspServerLayer : public stream::GlLayer
{
public:
    RtspServerLayer(
        stream::ControlServer&                 controlServer,
        stream::UdpServerEndpoint&             udpEndpoint,
        stream::GlFrameCapture&                capture,
        dc::ReceiverPort<stream::ServerStats>& statsRx,
        std::atomic<bool>&                     streaming)
        : m_controlServer(controlServer)
        , m_udpEndpoint(udpEndpoint)
        , m_capture(capture)
        , m_statsRx(statsRx)
        , m_streaming(streaming)
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
        ImGui::Begin("RTSP Server Control");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 4 — RTSP Control + UDP Data");
        ImGui::Separator();

        ImGui::Text("Control port : %d", g_settings->controlPort);
        ImGui::Text("Video port   : %d", g_settings->videoPort);
        ImGui::Text("Render FPS   : %.1f", m_renderFps);
        ImGui::Separator();

        if (m_controlServer.has_active_session())
        {
            auto cfg = m_controlServer.active_config();
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Session ACTIVE");
            ImGui::Text("Resolution   : %d x %d", cfg.width, cfg.height);
            ImGui::Text("FPS target   : %d", cfg.fps);
            ImGui::Text("Bitrate      : %.1f Mbps", cfg.bitrate / 1e6);
            ImGui::Text("FEC ratio    : %.0f%%", cfg.fec_ratio * 100.0f);
            ImGui::Text("Codec        : %s", cfg.codec.c_str());
            ImGui::Separator();
            ImGui::Text("Frames sent  : %llu", static_cast<unsigned long long>(m_stats.frames_sent));
            ImGui::Text("Pkts sent    : %llu", static_cast<unsigned long long>(m_udpEndpoint.packets_sent()));
            ImGui::Text("Data sent    : %.2f MB", static_cast<double>(m_stats.bytes_sent) / 1e6);
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.7f, 0.1f, 1.0f), "Waiting for client DESCRIBE/SETUP/PLAY ...");
            ImGui::TextDisabled("Control channel: TCP :%d", g_settings->controlPort);
            ImGui::TextDisabled("Video data:     UDP :%d", g_settings->videoPort);
        }

        ImGui::Separator();
        ImGui::TextDisabled("Protocol flow: DESCRIBE -> SETUP -> PLAY -> [stream] -> TEARDOWN");
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
        if (!m_streaming.load())
            return;

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

    stream::ControlServer&                 m_controlServer;
    stream::UdpServerEndpoint&             m_udpEndpoint;
    stream::GlFrameCapture&                m_capture;
    dc::ReceiverPort<stream::ServerStats>& m_statsRx;
    std::atomic<bool>&                     m_streaming;

    stream::ServerStats                   m_stats;
    float                                 m_renderFps   = 0.0f;
    std::chrono::steady_clock::time_point m_nextCapture = std::chrono::steady_clock::now();
};

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("rtsp_server_settings.json");

    Log::init(g_settings->logLevel, "rtsp_server.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("server");

    g_settings->targetFps = sanitize_fps(g_settings->targetFps);
    log->info("RTSP server starting (control={}, video={}, dest={})",
              g_settings->controlPort, g_settings->videoPort, g_settings->destIp);

    stream::GlApp app(g_settings->window);
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_frameReceiver.connect(g_frameSender);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::ServerStats> statsRx;
    statsRx.connect(g_statsSender);

    // --- UDP data-plane endpoint (starts paused; will send only when streaming flag is set) ---
    stream::UdpServerConfig udpCfg;
    udpCfg.dest_ip   = g_settings->destIp;
    udpCfg.dest_port = static_cast<uint16_t>(g_settings->videoPort);
    udpCfg.fec_ratio = g_settings->fecRatio;

    stream::UdpServerEndpoint udpEndpoint(udpCfg, g_frameReceiver, g_statsSender);
    udpEndpoint.start();

    // --- Capture & encode pipeline ---
    auto capture = std::make_unique<stream::GlFrameCapture>(g_frameSender);
    capture->set_after_deliver([&udpEndpoint] { udpEndpoint.notify_frame_available(); });

    int logicalW = 0, logicalH = 0;
    glfwGetWindowSize(app.window(), &logicalW, &logicalH);
    capture->set_target_size(logicalW, logicalH);

    auto encoder      = std::make_unique<stream::H264Encoder>();
    bool encoderReady = false;

    // These get updated when a session starts
    std::atomic<bool> streaming{false};
    int               activeBitrate = g_settings->bitrate;
    int               activeFps     = static_cast<int>(sanitize_fps(g_settings->targetFps));

    capture->set_encode_fn([&](stream::Frame& frame) -> bool {
        if (!streaming.load())
            return false;
        if (!encoderReady)
        {
            stream::EncoderConfig cfg;
            cfg.fps     = activeFps;
            cfg.bitrate = activeBitrate;
            if (!encoder->init(frame.width, frame.height, cfg))
                return false;
            encoderReady = true;
        }
        return encoder->encode(frame);
    });

    // --- Control-plane server ---
    stream::ControlServerConfig ctrlCfg;
    ctrlCfg.control_port = static_cast<uint16_t>(g_settings->controlPort);
    ctrlCfg.video_port   = static_cast<uint16_t>(g_settings->videoPort);

    stream::ControlServer controlServer(ctrlCfg);

    controlServer.set_on_session_start([&](const std::string& token, const stream::SessionConfig& cfg) {
        log->info("Session PLAY: {} ({}x{} @{}fps, {}bps, fec={:.0f}%)",
                  token, cfg.width, cfg.height, cfg.fps, cfg.bitrate, cfg.fec_ratio * 100.0f);
        activeBitrate = cfg.bitrate;
        activeFps     = cfg.fps;
        g_settings->targetFps = static_cast<float>(cfg.fps);

        // Reset encoder so it picks up new params
        encoderReady = false;
        encoder      = std::make_unique<stream::H264Encoder>();

        streaming.store(true);
    });

    controlServer.set_on_session_stop([&](const std::string& token) {
        log->info("Session TEARDOWN: {}", token);
        streaming.store(false);
    });

    controlServer.start();

    // --- UI layer ---
    app.pushLayer(std::make_unique<RtspServerLayer>(controlServer, udpEndpoint, *capture, statsRx, streaming));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    controlServer.stop();
    udpEndpoint.stop();
    capture.reset();

    SettingsRegistry::instance().saveJson("rtsp_server_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
