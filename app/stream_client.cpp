// ---------------------------------------------------------------------------
// stream_client.cpp — Phase 1: raw RGB frame streaming client
//
// Connects to a stream_server over TCP, receives [width][height][len][RGB]
// frames, uploads them to an OpenGL texture, and displays them live inside
// an ImGui window using ImGui::Image.
//
// Project infrastructure integrated:
//   - SettingsItem / SettingsRegistry  — server IP, port, log level (stream_client_settings.json)
//   - Log / ImGuiLogSink               — ANSI-color spdlog + in-window log panel
//   - dc::SenderPort / ReceiverPort    — zero-copy frame and stats handoff between threads
// ---------------------------------------------------------------------------
#ifndef _WIN32
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <unistd.h>
#  include <signal.h>
   using sock_t = int;
   static void close_sock(sock_t s) { ::close(s); }
#else
#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <ws2tcpip.h>
#  pragma comment(lib, "ws2_32.lib")
   using sock_t = SOCKET;
   static void close_sock(sock_t s) { ::closesocket(s); }
#endif

#include "settings/settings_item.hpp"
#include "settings/settings_registry.hpp"
#include "mylib/log.hpp"
#include "mylib/imgui_log_sink.hpp"
#include "mylib/data_container.hpp"

#include <nlohmann/json.hpp>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include <atomic>
#include <chrono>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Settings — loaded from stream_client_settings.json at startup
// ---------------------------------------------------------------------------
struct ClientSettings
{
    std::string serverIp   = "127.0.0.1";
    int         serverPort = 9999;
    std::string logLevel   = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ClientSettings, serverIp, serverPort, logLevel)
};
static SettingsItem<ClientSettings> g_settings("StreamClientSettings");

// ---------------------------------------------------------------------------
// Data ports — frame: net thread (producer) → render thread (consumer)
// ---------------------------------------------------------------------------
struct FrameData
{
    std::vector<uint8_t> pixels;
    int                  width  = 0;
    int                  height = 0;
};

static dc::Mempool<FrameData>    g_framePool(3);
static dc::SenderPort<FrameData> g_frameSender;   // owned by net thread

// ---------------------------------------------------------------------------
// Data ports — stats: net thread (producer) → render thread (consumer)
// ---------------------------------------------------------------------------
struct ClientStatsData
{
    uint64_t    bytesRecv  = 0;
    uint64_t    framesRecv = 0;
    float       fps        = 0.0f;
    bool        connected  = false;
    std::string status     = "Disconnected";
};

static dc::Mempool<ClientStatsData>    g_statsPool(2);
static dc::SenderPort<ClientStatsData> g_statsSender;   // owned by net thread

// ---------------------------------------------------------------------------
// Connection control (written by render thread, read by net thread)
// ---------------------------------------------------------------------------
static std::string       g_serverIp;    // written before g_doConnect.store(true)
static int               g_serverPort = 9999;
static std::atomic<bool> g_doConnect{false};
static std::atomic<bool> g_running{false};

// ---------------------------------------------------------------------------
// Helpers: receive exactly n bytes
// ---------------------------------------------------------------------------
static bool recv_all(sock_t fd, void* buf, size_t n)
{
    char* p = static_cast<char*>(buf);
    while (n > 0)
    {
#ifndef _WIN32
        ssize_t r = ::recv(fd, p, n, 0);
#else
        int r = ::recv(fd, p, static_cast<int>(n), 0);
#endif
        if (r <= 0) return false;
        p += r;
        n -= static_cast<size_t>(r);
    }
    return true;
}

// Push a stats snapshot to the render thread via data ports
static void deliverStats(uint64_t bytesRecv, uint64_t framesRecv, float fps,
                          bool connected, const std::string& status)
{
    ClientStatsData* slot = g_statsSender.reserve();
    if (!slot) return;
    slot->bytesRecv  = bytesRecv;
    slot->framesRecv = framesRecv;
    slot->fps        = fps;
    slot->connected  = connected;
    slot->status     = status;
    g_statsSender.deliver();
}

// ---------------------------------------------------------------------------
// Network thread: wait for connect request → receive frames → loop
// ---------------------------------------------------------------------------
static void netThread()
{
    auto log = Log::get("net");

    // Connect both senders to their mempools (net thread owns these)
    g_frameSender.connectMempool(g_framePool);
    g_statsSender.connectMempool(g_statsPool);

    while (g_running)
    {
        if (!g_doConnect.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // Snapshot connection target — written by render thread before doConnect was set
        const std::string ip   = g_serverIp;
        const int         port = g_serverPort;

        log->info("Connecting to {}:{}…", ip, port);
        deliverStats(0, 0, 0.0f, false, "Connecting to " + ip + ":" + std::to_string(port) + " …");

        sock_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
        {
            log->error("socket() failed");
            deliverStats(0, 0, 0.0f, false, "socket() failed");
            g_doConnect.store(false);
            continue;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(static_cast<uint16_t>(port));
        if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
        {
            close_sock(fd);
            log->error("Invalid IP: {}", ip);
            deliverStats(0, 0, 0.0f, false, "Invalid IP: " + ip);
            g_doConnect.store(false);
            continue;
        }

        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            close_sock(fd);
            log->warn("Connection refused to {}:{}", ip, port);
            deliverStats(0, 0, 0.0f, false, "Connection refused");
            g_doConnect.store(false);
            continue;
        }

        log->info("Connected to {}:{}", ip, port);
        deliverStats(0, 0, 0.0f, true, "Connected");

        using clock = std::chrono::steady_clock;
        auto lastTime = clock::now();

        uint64_t bytesRecv  = 0;
        uint64_t framesRecv = 0;
        float    fps        = 0.0f;

        while (g_running && g_doConnect.load())
        {
            // Read 12-byte header: [width 4B BE][height 4B BE][payload_size 4B BE]
            uint8_t hdr[12];
            if (!recv_all(fd, hdr, 12))
            {
                log->warn("recv header failed — server disconnected");
                break;
            }

            uint32_t w, h, len;
            std::memcpy(&w,   hdr + 0, 4); w   = ntohl(w);
            std::memcpy(&h,   hdr + 4, 4); h   = ntohl(h);
            std::memcpy(&len, hdr + 8, 4); len = ntohl(len);

            if (len == 0 || len > 64u * 1024u * 1024u)
            {
                log->error("Sanity check failed: len={} — disconnecting", len);
                break;
            }

            // Reserve a mempool slot and receive pixels directly into it (zero-copy)
            FrameData* slot = g_frameSender.reserve();
            if (slot)
            {
                slot->pixels.resize(len);
                if (!recv_all(fd, slot->pixels.data(), len))
                {
                    log->warn("recv pixels failed — server disconnected");
                    g_frameSender.deliver();   // deliver partial (size guard above protects)
                    break;
                }
                slot->width  = static_cast<int>(w);
                slot->height = static_cast<int>(h);
                g_frameSender.deliver();
            }
            else
            {
                // Pool exhausted: drain the socket to keep the stream in sync
                log->warn("Frame pool exhausted — draining {} bytes", len);
                std::vector<uint8_t> drain(len);
                if (!recv_all(fd, drain.data(), len)) break;
            }

            // FPS measurement (exponential moving average, α = 0.1)
            auto  now = clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;
            bytesRecv  += 12 + len;
            framesRecv++;
            if (dt > 0.0f)
                fps = fps * 0.9f + (1.0f / dt) * 0.1f;

            deliverStats(bytesRecv, framesRecv, fps, true, "Connected");

            if (framesRecv % 300 == 0)
                log->debug("Received {} frames ({:.2f} MB)", framesRecv,
                           static_cast<double>(bytesRecv) / 1e6);
        }

        close_sock(fd);
        log->info("Disconnected from {}:{}", ip, port);
        deliverStats(0, 0, 0.0f, false, "Disconnected");
        g_doConnect.store(false);
    }

    log->info("Net thread exited");
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main()
{
#ifdef _WIN32
    WSADATA wsa{};
    WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    ::signal(SIGPIPE, SIG_IGN);
#endif

    // Load settings (missing file is silently skipped; defaults apply)
    SettingsRegistry::instance().loadJson("stream_client_settings.json");

    // Initialize logging with ANSI colors + in-window ImGui sink
    Log::init(g_settings->logLevel, "stream_client.log");
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("client");
    log->info("Stream client starting (defaultServer={}:{}, logLevel={})",
              g_settings->serverIp, g_settings->serverPort, g_settings->logLevel);

    // Initialize connection target from settings
    g_serverIp   = g_settings->serverIp;
    g_serverPort = g_settings->serverPort;

    if (!glfwInit()) return 1;

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char* glslVersion = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    const char* glslVersion = "#version 130";
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Stream Client", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glslVersion);

    // Stream display texture
    GLuint streamTex = 0;
    int    texW = 0, texH = 0;
    glGenTextures(1, &streamTex);
    glBindTexture(GL_TEXTURE_2D, streamTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Frame and stats receivers (render thread owns these)
    dc::ReceiverPort<FrameData>       frameReceiver;
    dc::ReceiverPort<ClientStatsData> statsReceiver;
    frameReceiver.connect(g_frameSender);
    statsReceiver.connect(g_statsSender);

    g_running.store(true);
    std::thread netThr(netThread);

    // UI state — pre-filled from settings
    char ipBuf[256];
    std::strncpy(ipBuf, g_settings->serverIp.c_str(), sizeof(ipBuf) - 1);
    ipBuf[sizeof(ipBuf) - 1] = '\0';
    int portBuf = g_settings->serverPort;

    // Stats snapshot (updated each frame from data ports)
    uint64_t    bytesRecv  = 0;
    uint64_t    framesRecv = 0;
    float       recvFps    = 0.0f;
    bool        connected  = false;
    std::string status     = "Disconnected";

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Promote latest frame from net thread
        frameReceiver.update();
        if (frameReceiver.hasNewData())
        {
            const FrameData* f = frameReceiver.getData();
            if (f && !f->pixels.empty())
            {
                glBindTexture(GL_TEXTURE_2D, streamTex);
                if (f->width != texW || f->height != texH)
                {
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                        f->width, f->height, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, f->pixels.data());
                    texW = f->width;
                    texH = f->height;
                }
                else
                {
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        f->width, f->height,
                        GL_RGB, GL_UNSIGNED_BYTE, f->pixels.data());
                }
            }
        }
        frameReceiver.cleanup();

        // Pull latest stats from net thread
        statsReceiver.update();
        if (statsReceiver.hasNewData())
        {
            const ClientStatsData* s = statsReceiver.getData();
            bytesRecv  = s->bytesRecv;
            framesRecv = s->framesRecv;
            recvFps    = s->fps;
            connected  = s->connected;
            status     = s->status;
        }
        statsReceiver.cleanup();

        // --- ImGui frame --------------------------------------------------
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Full-screen DockSpace
        {
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);
            ImGuiWindowFlags flags =
                ImGuiWindowFlags_NoTitleBar  | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize    | ImGuiWindowFlags_NoMove     |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("##DockSpace", nullptr, flags);
            ImGui::PopStyleVar();
            ImGui::DockSpace(ImGui::GetID("MainDS"), ImVec2(0, 0),
                ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::End();
        }

        // --- Connection panel ---------------------------------------------
        ImGui::Begin("Connection");

        bool isConnecting = g_doConnect.load() && !connected;

        ImGui::InputText("Server IP", ipBuf, sizeof(ipBuf),
            connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);
        ImGui::InputInt("Port", &portBuf,
            connected || isConnecting ? ImGuiInputTextFlags_ReadOnly : 0);

        if (!connected && !isConnecting)
        {
            if (ImGui::Button("Connect"))
            {
                g_serverIp   = ipBuf;
                g_serverPort = portBuf;
                log->info("Requesting connect to {}:{}", g_serverIp, g_serverPort);
                g_doConnect.store(true);
            }
        }
        else if (isConnecting)
        {
            ImGui::BeginDisabled();
            ImGui::Button("Connecting…");
            ImGui::EndDisabled();
        }
        else
        {
            if (ImGui::Button("Disconnect"))
            {
                log->info("Disconnect requested");
                g_doConnect.store(false);
            }
        }

        ImGui::Separator();
        if (connected)
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f), "Status: %s", status.c_str());
        else
            ImGui::TextColored(ImVec4(1.0f, 0.55f, 0.1f, 1.0f), "Status: %s", status.c_str());

        ImGui::Text("FPS recv   : %.1f", recvFps);
        ImGui::Text("Frames     : %llu", static_cast<unsigned long long>(framesRecv));
        ImGui::Text("Data recv  : %.2f MB", static_cast<double>(bytesRecv) / 1e6);
        if (texW > 0)
            ImGui::Text("Resolution : %d x %d", texW, texH);
        ImGui::Text("Log level  : %s", g_settings->logLevel.c_str());
        ImGui::End();

        // --- Video Stream panel -------------------------------------------
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Video Stream");
        ImGui::PopStyleVar();

        if (texW > 0 && texH > 0)
        {
            ImVec2 avail  = ImGui::GetContentRegionAvail();
            float  aspect = static_cast<float>(texW) / static_cast<float>(texH);
            float  dw     = avail.x;
            float  dh     = dw / aspect;
            if (dh > avail.y) { dh = avail.y; dw = dh * aspect; }
            ImGui::SetCursorPos(ImVec2(
                (avail.x - dw) * 0.5f + ImGui::GetCursorPosX(),
                (avail.y - dh) * 0.5f + ImGui::GetCursorPosY()));
            ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(streamTex)),
                ImVec2(dw, dh));
        }
        else
        {
            ImVec2 avail = ImGui::GetContentRegionAvail();
            ImGui::SetCursorPos(ImVec2(avail.x * 0.5f - 100.0f, avail.y * 0.5f));
            ImGui::TextDisabled("No stream — connect to a server");
        }
        ImGui::End();

        // --- Log panel ----------------------------------------------------
        imguiSink->draw("Log");

        // --- Render -------------------------------------------------------
        ImGui::Render();
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0.08f, 0.08f, 0.10f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // --- Shutdown ---------------------------------------------------------
    log->info("Shutting down");
    g_running.store(false);
    g_doConnect.store(false);
    netThr.join();

    glDeleteTextures(1, &streamTex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    // Persist last-used connection settings for next run
    g_settings->serverIp   = ipBuf;
    g_settings->serverPort = portBuf;
    SettingsRegistry::instance().saveJson("stream_client_settings.json");
    log->info("Settings saved to stream_client_settings.json. Exiting.");

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
