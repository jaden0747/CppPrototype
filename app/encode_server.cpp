// ---------------------------------------------------------------------------
// encode_server.cpp — Phase 2: H.264-encoded frame streaming server
//
// Same structure as stream_server but the capture hook encodes each raw RGB
// frame to H.264 NAL bytes (libx264, ultrafast/zerolatency) before TCP send.
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
#include "stream/stream_server.hpp"
#include "stream/tcp_socket.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <nlohmann/json.hpp>

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>

struct EncodeServerSettings
{
    int         port      = 9999;
    float       targetFps = 30.0f;
    int         bitrate   = 4'000'000;
    std::string logLevel  = "info";
    // vsync=false: server doesn't need smooth display; vsync blocks the render
    // thread and eats into the encode time budget.
    // max_render_fps must be well above targetFps so the onRender rate-limiter
    // can fire at the right moment — render loop overhead eats into the budget.
    stream::GlAppConfig
        window{"Encode Server (H.264)", 1280, 720, false, true, {0.10f, 0.10f, 0.14f, 1.0f}, "", 16.0f, 120.0f};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(EncodeServerSettings, port, targetFps, bitrate, logLevel, window)
};

static SettingsItem<EncodeServerSettings> g_settings("EncodeServerSettings");

static dc::Mempool<stream::Frame>      g_framePool(3);
static dc::SenderPort<stream::Frame>   g_frameSender;
static dc::ReceiverPort<stream::Frame> g_frameReceiver;

static dc::Mempool<stream::ServerStats>    g_statsPool(2);
static dc::SenderPort<stream::ServerStats> g_statsSender;

namespace
{

float sanitize_target_fps(float fps)
{
    if (fps < 1.0f)
        return 1.0f;
    if (fps > 240.0f)
        return 240.0f;
    return fps;
}

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

// ---- Layer -----------------------------------------------------------------

class EncodeServerLayer : public stream::GlLayer
{
public:
    EncodeServerLayer(
        stream::StreamServerEndpoint&          endpoint,
        stream::GlFrameCapture&                capture,
        dc::ReceiverPort<stream::ServerStats>& statsReceiver)
        : m_endpoint(endpoint)
        , m_capture(capture)
        , m_statsReceiver(statsReceiver)
    {
    }

    void onUpdate(float dt) override
    {
        m_statsReceiver.update();
        if (m_statsReceiver.hasNewData())
        {
            if (const auto* s = m_statsReceiver.getData())
                m_stats = *s;
        }
        m_statsReceiver.cleanup();

        if (dt > 0.0f)
            m_renderFps = m_renderFps * 0.95f + (1.0f / dt) * 0.05f;

        SettingsRegistry::instance().tickAutoSave();
    }

    void onImGui() override
    {
        ImGui::Begin("Encode Server Control");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 2 — H.264 Encoding");
        ImGui::Separator();
        ImGui::Text("Render FPS   : %.1f", m_renderFps);
        ImGui::Text("Stream FPS   : %.1f target", g_settings->targetFps);
        ImGui::Text("Bitrate      : %.1f Mbps", g_settings->bitrate / 1e6);
        ImGui::Text("Port         : %d", g_settings->port);
        ImGui::Separator();
        if (m_stats.connected)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Connected: %s", m_stats.peer.c_str());
            ImGui::Text("Frames sent  : %llu", static_cast<unsigned long long>(m_stats.frames_sent));
            ImGui::Text("Data sent    : %.2f MB", static_cast<double>(m_stats.bytes_sent) / 1e6);
            if (m_stats.frames_sent > 0 && m_capture.preview_ready())
            {
                const double raw_mb_per_s = static_cast<double>(m_capture.preview_width()) *
                                            static_cast<double>(m_capture.preview_height()) * 3.0 *
                                            static_cast<double>(g_settings->targetFps) / 1e6;
                ImGui::Text("Raw would be : %.1f MB/s", raw_mb_per_s);
                const double enc_mb_per_s = g_settings->bitrate / 8.0 / 1e6;
                ImGui::Text("Encoded      : ~%.1f MB/s (%.0fx smaller)", enc_mb_per_s, raw_mb_per_s / enc_mb_per_s);
            }
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.1f, 1.0f), "Waiting for client on :%d ...", g_settings->port);
        }
        ImGui::Separator();
        ImGui::TextDisabled("Codec: libx264, preset=ultrafast, tune=zerolatency");
        ImGui::TextDisabled("Protocol: [width][height][format][H264][len][NAL bytes]");
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

        m_lastCaptureTime = now;
        m_nextCapture     = now + captureInterval();
        if (!m_capture.capture(fbW, fbH))
            Log::get("server")->warn("Capture/encode skipped this frame");
    }

    std::chrono::steady_clock::time_point nextWakeup() const override
    {
        return m_nextCapture;
    }

private:
    std::chrono::nanoseconds captureInterval() const
    {
        const double secs = 1.0 / sanitize_target_fps(g_settings->targetFps);
        return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::duration<double>(secs));
    }

    stream::StreamServerEndpoint&          m_endpoint;
    stream::GlFrameCapture&                m_capture;
    dc::ReceiverPort<stream::ServerStats>& m_statsReceiver;

    stream::ServerStats                   m_stats;
    float                                 m_renderFps       = 0.0f;
    std::chrono::steady_clock::time_point m_lastCaptureTime = std::chrono::steady_clock::now();
    std::chrono::steady_clock::time_point m_nextCapture     = std::chrono::steady_clock::now();
};

} // namespace

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("encode_server_settings.json");

    Log::init(g_settings->logLevel, "encode_server.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("server");

    g_settings->targetFps = sanitize_target_fps(g_settings->targetFps);
    log->info(
        "Encode server starting (port={}, fps={}, bitrate={}bps, logLevel={})",
        g_settings->port,
        g_settings->targetFps,
        g_settings->bitrate,
        g_settings->logLevel);

    stream::GlApp app(g_settings->window);
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

    // On retina displays glfwGetFramebufferSize returns 2× the logical size.
    int logicalW = 0, logicalH = 0;
    glfwGetWindowSize(app.window(), &logicalW, &logicalH);
    capture->set_target_size(logicalW, logicalH);

    auto      encoder       = std::make_unique<stream::H264Encoder>();
    bool      encoderReady  = false;
    const int targetBitrate = g_settings->bitrate;
    const int targetFpsInt  = static_cast<int>(sanitize_target_fps(g_settings->targetFps));

    capture->set_encode_fn(
        [&](stream::Frame& frame) -> bool
        {
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

    app.pushLayer(std::make_unique<EncodeServerLayer>(endpoint, *capture, statsReceiver));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    endpoint.stop();
    capture.reset();

    SettingsRegistry::instance().saveJson("encode_server_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
