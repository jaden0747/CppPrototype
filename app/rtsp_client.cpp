// ---------------------------------------------------------------------------
// rtsp_client.cpp — Phase 4: RTSP-like control negotiation + UDP/FEC receiver
//
// The client performs a DESCRIBE → SETUP → PLAY handshake with the server's
// control channel, then receives H.264 video via UDP/FEC and decodes it.
// The control channel negotiates codec, resolution, bitrate, and FEC ratio
// before any video data flows.
// ---------------------------------------------------------------------------
#include "mylib/data_container.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/log.hpp"
#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "stream/control_client.hpp"
#include "stream/control_protocol.hpp"
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
#include <cstring>
#include <memory>
#include <string>
#include <thread>

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

struct RtspClientSettings
{
    std::string serverIp    = "127.0.0.1";
    int         controlPort = 48010;
    int         listenPort  = 9998;
    int         width       = 1280;
    int         height      = 720;
    int         fps         = 30;
    int         bitrate     = 4'000'000;
    float       fecRatio    = 0.25f;
    std::string logLevel    = "info";
    stream::GlAppConfig
        window{"RTSP Client (Phase 4)", 1280, 720, true, true, {0.08f, 0.08f, 0.10f, 1.0f}};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        RtspClientSettings, serverIp, controlPort, listenPort, width, height, fps, bitrate, fecRatio, logLevel, window)
};

static SettingsItem<RtspClientSettings> g_settings("RtspClientSettings");

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

// ---------------------------------------------------------------------------
// Session state machine
// ---------------------------------------------------------------------------

enum class SessionState
{
    Disconnected,
    Connecting,
    Described,
    Ready,     // SETUP done
    Streaming, // PLAY sent
    Error,
};

const char* state_label(SessionState s)
{
    switch (s)
    {
    case SessionState::Disconnected: return "Disconnected";
    case SessionState::Connecting: return "Connecting...";
    case SessionState::Described: return "Described";
    case SessionState::Ready: return "Ready (session created)";
    case SessionState::Streaming: return "Streaming";
    case SessionState::Error: return "Error";
    }
    return "Unknown";
}

// ---- Layer -----------------------------------------------------------------

class RtspClientLayer : public stream::GlLayer
{
public:
    RtspClientLayer(
        stream::UdpClientEndpoint&             udpEndpoint,
        stream::GlFrameTexture&                texture,
        dc::ReceiverPort<stream::Frame>&       frameRx,
        dc::ReceiverPort<stream::ClientStats>& statsRx)
        : m_udpEndpoint(udpEndpoint)
        , m_texture(texture)
        , m_frameRx(frameRx)
        , m_statsRx(statsRx)
    {
        std::strncpy(m_ipBuf, g_settings->serverIp.c_str(), sizeof(m_ipBuf) - 1);
        m_ipBuf[sizeof(m_ipBuf) - 1] = '\0';
        m_controlPort                 = g_settings->controlPort;
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
        ImGui::Begin("RTSP Client Control");
        ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Phase 4 — RTSP Negotiation + UDP Stream");
        ImGui::Separator();

        bool isStreaming = (m_state == SessionState::Streaming);

        ImGui::InputText("Server IP", m_ipBuf, sizeof(m_ipBuf), isStreaming ? ImGuiInputTextFlags_ReadOnly : 0);
        ImGui::InputInt("Control Port", &m_controlPort, isStreaming ? ImGuiInputTextFlags_ReadOnly : 0);
        ImGui::Separator();

        if (m_state == SessionState::Disconnected || m_state == SessionState::Error)
        {
            if (ImGui::Button("Connect & Play"))
            {
                do_connect_and_play();
            }
        }
        else if (isStreaming)
        {
            if (ImGui::Button("Teardown"))
            {
                do_teardown();
            }
        }
        else
        {
            ImGui::BeginDisabled();
            ImGui::Button("Working...");
            ImGui::EndDisabled();
        }

        ImGui::Separator();

        // State display
        ImVec4 stateColor = isStreaming ? ImVec4(0.2f, 1.0f, 0.2f, 1.0f) : ImVec4(1.0f, 0.7f, 0.1f, 1.0f);
        ImGui::TextColored(stateColor, "State: %s", state_label(m_state));

        if (!m_errorMsg.empty())
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "Error: %s", m_errorMsg.c_str());

        if (isStreaming)
        {
            ImGui::Separator();
            ImGui::Text("Session      : %s", m_sessionToken.c_str());
            ImGui::Text("Negotiated   : %dx%d @%dfps, %.1f Mbps",
                        m_actualConfig.width, m_actualConfig.height,
                        m_actualConfig.fps, m_actualConfig.bitrate / 1e6);
            ImGui::Text("FEC          : %.0f%%", m_actualConfig.fec_ratio * 100.0f);
            ImGui::Separator();
            ImGui::Text("FPS recv     : %.1f", m_stats.fps);
            ImGui::Text("Frames       : %llu", static_cast<unsigned long long>(m_stats.frames_received));
            ImGui::Text("Data recv    : %.2f MB", static_cast<double>(m_stats.bytes_received) / 1e6);
            if (m_texture.ready())
                ImGui::Text("Resolution   : %d x %d", m_texture.width(), m_texture.height());
        }

        if (!m_caps.codecs.empty())
        {
            ImGui::Separator();
            ImGui::TextDisabled("Server caps: max %dx%d @%dfps, port %d",
                                m_caps.max_width, m_caps.max_height, m_caps.max_fps, m_caps.video_port);
            std::string codecs_str;
            for (const auto& c : m_caps.codecs)
                codecs_str += c + " ";
            ImGui::TextDisabled("Codecs: %s", codecs_str.c_str());
        }

        ImGui::End();

        // Video window
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Video Stream (RTSP)");
        ImGui::PopStyleVar();

        if (m_texture.ready())
            draw_stream_texture(m_texture);
        else
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 130.0f, avail.y * 0.5f));
            ImGui::TextDisabled("No stream — connect to RTSP server");
        }
        ImGui::End();
    }

    const char* ip() const { return m_ipBuf; }
    int         controlPort() const { return m_controlPort; }

private:
    void do_connect_and_play()
    {
        m_errorMsg.clear();
        m_state = SessionState::Connecting;

        // Run negotiation in a background thread to avoid blocking the UI
        std::thread([this] {
            auto log = Log::get("client");

            stream::ControlClientConfig cfg;
            cfg.server_ip    = m_ipBuf;
            cfg.control_port = static_cast<uint16_t>(m_controlPort);

            stream::ControlClient client;
            if (!client.connect(cfg))
            {
                m_errorMsg = "Control connection refused";
                m_state    = SessionState::Error;
                return;
            }

            // DESCRIBE
            if (!client.describe(m_caps))
            {
                m_errorMsg = "DESCRIBE failed";
                m_state    = SessionState::Error;
                client.disconnect();
                return;
            }
            m_state = SessionState::Described;
            log->info("Server caps: max {}x{} @{}fps, codecs={}, video_port={}",
                      m_caps.max_width, m_caps.max_height, m_caps.max_fps,
                      m_caps.codecs.empty() ? "none" : m_caps.codecs[0], m_caps.video_port);

            // SETUP
            stream::SessionConfig desired;
            desired.width     = g_settings->width;
            desired.height    = g_settings->height;
            desired.fps       = g_settings->fps;
            desired.bitrate   = g_settings->bitrate;
            desired.fec_ratio = g_settings->fecRatio;

            if (!client.setup(desired, m_sessionToken, m_actualConfig))
            {
                m_errorMsg = "SETUP failed";
                m_state    = SessionState::Error;
                client.disconnect();
                return;
            }
            m_state = SessionState::Ready;
            log->info("Session: {} ({}x{} @{}fps)", m_sessionToken,
                      m_actualConfig.width, m_actualConfig.height, m_actualConfig.fps);

            // Start UDP receiver before sending PLAY
            if (!m_udpEndpoint.is_running())
            {
                stream::UdpClientConfig udpCfg;
                udpCfg.listen_port = static_cast<uint16_t>(g_settings->listenPort);
                m_udpEndpoint.start(udpCfg);
            }

            // PLAY
            if (!client.play(m_sessionToken))
            {
                m_errorMsg = "PLAY failed";
                m_state    = SessionState::Error;
                client.disconnect();
                return;
            }

            m_state = SessionState::Streaming;
            log->info("Streaming started");

            // Keep control connection alive (stored for teardown)
            m_controlClient.reset(new stream::ControlClient(std::move(client)));
        }).detach();
    }

    void do_teardown()
    {
        auto log = Log::get("client");
        if (m_controlClient && !m_sessionToken.empty())
        {
            m_controlClient->teardown(m_sessionToken);
            m_controlClient->disconnect();
            m_controlClient.reset();
            log->info("Teardown sent");
        }
        m_udpEndpoint.stop();
        m_state        = SessionState::Disconnected;
        m_sessionToken.clear();
    }

    stream::UdpClientEndpoint&             m_udpEndpoint;
    stream::GlFrameTexture&                m_texture;
    dc::ReceiverPort<stream::Frame>&       m_frameRx;
    dc::ReceiverPort<stream::ClientStats>& m_statsRx;

    stream::ClientStats                    m_stats;
    SessionState                           m_state = SessionState::Disconnected;
    std::string                            m_errorMsg;
    std::string                            m_sessionToken;
    stream::ServerCapabilities             m_caps;
    stream::SessionConfig                  m_actualConfig;
    std::unique_ptr<stream::ControlClient> m_controlClient;

    char m_ipBuf[256]  = {};
    int  m_controlPort = 48010;
};

} // namespace

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    stream::ignore_sigpipe();

    SettingsRegistry::instance().loadJson("rtsp_client_settings.json");

    Log::init(g_settings->logLevel, "rtsp_client.log");
    stream::install_ffmpeg_log_bridge();
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("client");
    log->info("RTSP client starting (server={}:{}, listen={})",
              g_settings->serverIp, g_settings->controlPort, g_settings->listenPort);

    stream::GlApp app(g_settings->window);
    if (!app.valid())
        return 1;

    g_frameSender.connectMempool(g_framePool);
    g_statsSender.connectMempool(g_statsPool);

    dc::ReceiverPort<stream::Frame>       frameRx;
    dc::ReceiverPort<stream::ClientStats> statsRx;
    frameRx.connect(g_frameSender);
    statsRx.connect(g_statsSender);

    stream::UdpClientEndpoint udpEndpoint(g_frameSender, g_statsSender);

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

    auto* clientLayer = new RtspClientLayer(udpEndpoint, *streamTexture, frameRx, statsRx);
    app.pushLayer(std::unique_ptr<stream::GlLayer>(clientLayer));
    app.pushLayer(std::make_unique<stream::ImGuiLogLayer>(imguiSink));

    app.run();

    log->info("Shutting down");
    udpEndpoint.stop();
    streamTexture.reset();

    g_settings->serverIp    = clientLayer->ip();
    g_settings->controlPort = clientLayer->controlPort();
    SettingsRegistry::instance().saveJson("rtsp_client_settings.json");
    log->info("Settings saved. Exiting.");
    return 0;
}
