// ---------------------------------------------------------------------------
// stream_server.cpp — Phase 1: raw RGB frame streaming server
//
// Renders an ImGui window, captures its own OpenGL framebuffer with
// glReadPixels after each frame, and streams the pixels to a connected
// client over TCP using a simple [width][height][len][pixels] protocol.
//
// Project infrastructure integrated:
//   - SettingsItem / SettingsRegistry  — port and log level (stream_server_settings.json)
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
#include <condition_variable>
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// ---------------------------------------------------------------------------
// Settings — loaded from stream_server_settings.json at startup
// ---------------------------------------------------------------------------
struct ServerSettings
{
    int         port     = 9999;
    std::string logLevel = "info";
    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ServerSettings, port, logLevel)
};
static SettingsItem<ServerSettings> g_settings("StreamServerSettings");

// ---------------------------------------------------------------------------
// Data ports — frame: render thread (producer) → net thread (consumer)
// ---------------------------------------------------------------------------
struct FrameData
{
    std::vector<uint8_t> pixels;
    int                  width  = 0;
    int                  height = 0;
};

static dc::Mempool<FrameData>      g_framePool(3);
static dc::SenderPort<FrameData>   g_frameSender;
static dc::ReceiverPort<FrameData> g_frameReceiver;  // owned by net thread

// Condvar lets net thread sleep until a frame arrives (avoids busy-wait)
static std::mutex              g_frameCvMtx;
static std::condition_variable g_frameCv;

// ---------------------------------------------------------------------------
// Data ports — stats: net thread (producer) → render thread (consumer)
// ---------------------------------------------------------------------------
struct NetStatsData
{
    uint64_t    bytesSent  = 0;
    uint64_t    framesSent = 0;
    bool        connected  = false;
    std::string clientAddr;
};

static dc::Mempool<NetStatsData>    g_statsPool(2);
static dc::SenderPort<NetStatsData> g_statsSender;   // owned by net thread

// ---------------------------------------------------------------------------
static std::atomic<bool> g_running{false};

// ---------------------------------------------------------------------------
// Helpers: send exactly n bytes
// ---------------------------------------------------------------------------
static bool send_all(sock_t fd, const void* buf, size_t n)
{
    const char* p = static_cast<const char*>(buf);
    while (n > 0)
    {
#ifndef _WIN32
        ssize_t s = ::send(fd, p, n, 0);
#else
        int s = ::send(fd, p, static_cast<int>(n), 0);
#endif
        if (s <= 0) return false;
        p += s;
        n -= static_cast<size_t>(s);
    }
    return true;
}

// Wire format: [4B width BE][4B height BE][4B payload_size BE][pixels RGB]
static bool send_frame(sock_t fd, const uint8_t* pixels, int w, int h)
{
    uint32_t bw  = htonl(static_cast<uint32_t>(w));
    uint32_t bh  = htonl(static_cast<uint32_t>(h));
    uint32_t bsz = htonl(static_cast<uint32_t>(w * h * 3));
    uint8_t  hdr[12];
    std::memcpy(hdr + 0, &bw,  4);
    std::memcpy(hdr + 4, &bh,  4);
    std::memcpy(hdr + 8, &bsz, 4);
    return send_all(fd, hdr, 12) && send_all(fd, pixels, static_cast<size_t>(w * h * 3));
}

// Push a stats snapshot to the render thread via data ports
static void deliverStats(uint64_t bytesSent, uint64_t framesSent,
                          bool connected, const std::string& clientAddr = {})
{
    NetStatsData* slot = g_statsSender.reserve();
    if (!slot) return;
    slot->bytesSent  = bytesSent;
    slot->framesSent = framesSent;
    slot->connected  = connected;
    slot->clientAddr = clientAddr;
    g_statsSender.deliver();
}

// ---------------------------------------------------------------------------
// Network thread: listen → accept → stream frames → loop
// ---------------------------------------------------------------------------
static void netThread()
{
    auto log = Log::get("net");

    // Connect stats sender to its mempool (net thread owns this sender)
    g_statsSender.connectMempool(g_statsPool);

    const auto port = static_cast<uint16_t>(g_settings->port);

    sock_t srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { log->error("socket() failed"); return; }

    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(port);

    if (::bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        log->error("bind() failed on port {}", port);
        close_sock(srv);
        return;
    }
    ::listen(srv, 1);
    log->info("Listening on TCP :{}", port);

    while (g_running)
    {
        // Non-blocking accept with 1 s timeout so we can re-check g_running
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(srv, &fds);
        timeval tv{1, 0};
        if (::select(static_cast<int>(srv) + 1, &fds, nullptr, nullptr, &tv) <= 0)
            continue;

        sockaddr_in caddr{};
        socklen_t   clen   = sizeof(caddr);
        sock_t      client = ::accept(srv, reinterpret_cast<sockaddr*>(&caddr), &clen);
        if (client < 0) continue;

        char clientIp[INET_ADDRSTRLEN] = {};
        ::inet_ntop(AF_INET, &caddr.sin_addr, clientIp, sizeof(clientIp));
        std::string addrStr = std::string(clientIp) + ":" + std::to_string(ntohs(caddr.sin_port));
        log->info("Client connected: {}", addrStr);

        deliverStats(0, 0, true, addrStr);

        uint64_t bytesSent  = 0;
        uint64_t framesSent = 0;

        while (g_running)
        {
            // Sleep until the render thread notifies us (or timeout after 200 ms)
            {
                std::unique_lock<std::mutex> lk(g_frameCvMtx);
                g_frameCv.wait_for(lk, std::chrono::milliseconds(200));
            }
            if (!g_running) break;

            // Promote any pending frame to active
            g_frameReceiver.update();
            if (!g_frameReceiver.hasNewData())
            {
                g_frameReceiver.cleanup();
                continue;
            }

            const FrameData* f = g_frameReceiver.getData();
            if (!f || f->pixels.empty() || f->width <= 0 || f->height <= 0)
            {
                g_frameReceiver.cleanup();
                continue;
            }

            if (!send_frame(client, f->pixels.data(), f->width, f->height))
            {
                log->warn("send_frame failed — client disconnected");
                g_frameReceiver.cleanup();
                break;
            }

            bytesSent  += 12 + static_cast<uint64_t>(f->width * f->height * 3);
            framesSent++;
            g_frameReceiver.cleanup();

            deliverStats(bytesSent, framesSent, true, addrStr);

            if (framesSent % 300 == 0)
                log->debug("Sent {} frames ({:.2f} MB)", framesSent,
                           static_cast<double>(bytesSent) / 1e6);
        }

        close_sock(client);
        log->info("Client disconnected: {}", addrStr);
        deliverStats(0, 0, false);
    }

    close_sock(srv);
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
    SettingsRegistry::instance().loadJson("stream_server_settings.json");

    // Initialize logging with ANSI colors + in-window ImGui sink
    Log::init(g_settings->logLevel, "stream_server.log");
    auto imguiSink = std::make_shared<ImGuiLogSink_mt>();
    imguiSink->set_pattern("%^[%H:%M:%S.%e] [%n] [%l] %v%$");
    Log::addSink(imguiSink);
    auto log = Log::get("server");
    log->info("Stream server starting (port={}, logLevel={})",
              g_settings->port, g_settings->logLevel);

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

    GLFWwindow* window = glfwCreateWindow(1280, 720, "Stream Server", nullptr, nullptr);
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

    // Preview texture (shows what is being streamed)
    GLuint previewTex   = 0;
    bool   previewReady = false;
    int    previewW     = 0;
    int    previewH     = 0;
    glGenTextures(1, &previewTex);
    glBindTexture(GL_TEXTURE_2D, previewTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Wire up frame pipeline: render thread → Mempool → SenderPort → ReceiverPort → net thread
    g_frameSender.connectMempool(g_framePool);
    g_frameReceiver.connect(g_frameSender);

    // Stats receiver lives on the render thread
    dc::ReceiverPort<NetStatsData> statsReceiver;
    statsReceiver.connect(g_statsSender);

    g_running.store(true);
    std::thread netThr(netThread);

    // FPS tracking
    using clock = std::chrono::steady_clock;
    auto  lastFrameTime = clock::now();
    float renderFps     = 0.0f;

    // Stats snapshot (updated each frame from data ports)
    uint64_t    bytesSent  = 0;
    uint64_t    framesSent = 0;
    bool        connected  = false;
    std::string clientAddr;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Pull latest stats from net thread (zero-copy via data ports)
        statsReceiver.update();
        if (statsReceiver.hasNewData())
        {
            const NetStatsData* s = statsReceiver.getData();
            bytesSent  = s->bytesSent;
            framesSent = s->framesSent;
            connected  = s->connected;
            clientAddr = s->clientAddr;
        }
        statsReceiver.cleanup();

        // Render FPS (exponential moving average, α = 0.05)
        auto  now = clock::now();
        float dt  = std::chrono::duration<float>(now - lastFrameTime).count();
        if (dt > 0.0f) renderFps = renderFps * 0.95f + (1.0f / dt) * 0.05f;
        lastFrameTime = now;

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

        // --- Server Control panel -----------------------------------------
        ImGui::Begin("Server Control");
        ImGui::Text("Render FPS : %.1f", renderFps);
        ImGui::Text("Port       : %d", g_settings->port);
        ImGui::Text("Log level  : %s", g_settings->logLevel.c_str());
        ImGui::Separator();
        if (connected)
        {
            ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                "Connected: %s", clientAddr.c_str());
            ImGui::Text("Frames sent : %llu",
                static_cast<unsigned long long>(framesSent));
            ImGui::Text("Data sent   : %.2f MB",
                static_cast<double>(bytesSent) / 1e6);
        }
        else
        {
            ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.1f, 1.0f),
                "Waiting for client on :%d …", g_settings->port);
        }
        ImGui::Separator();
        ImGui::TextDisabled("Protocol: [4B width][4B height][4B len][RGB]");
        ImGui::End();

        // --- Captured Preview panel ---------------------------------------
        ImGui::Begin("Captured Preview");
        ImGui::TextDisabled("What the server is streaming right now:");
        if (previewReady)
        {
            ImVec2 avail  = ImGui::GetContentRegionAvail();
            float  aspect = previewH > 0
                ? static_cast<float>(previewW) / static_cast<float>(previewH)
                : 16.0f / 9.0f;
            float dw = avail.x;
            float dh = dw / aspect;
            if (dh > avail.y) { dh = avail.y; dw = dh * aspect; }
            ImGui::Image(static_cast<ImTextureID>(static_cast<uintptr_t>(previewTex)),
                ImVec2(dw, dh));
        }
        else
        {
            ImGui::Text("Waiting for first frame…");
        }
        ImGui::End();

        // --- Log panel ----------------------------------------------------
        imguiSink->draw("Log");

        // --- Render -------------------------------------------------------
        ImGui::Render();
        int fbW, fbH;
        glfwGetFramebufferSize(window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0.10f, 0.10f, 0.14f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // --- Capture framebuffer AFTER render, BEFORE swap ----------------
        {
            // Reserve a slot from the mempool for zero-copy frame delivery
            FrameData* slot = g_frameSender.reserve();
            if (slot)
            {
                slot->pixels.resize(static_cast<size_t>(fbW * fbH * 3));
                glReadPixels(0, 0, fbW, fbH, GL_RGB, GL_UNSIGNED_BYTE,
                    slot->pixels.data());

                // OpenGL origin is bottom-left; flip to standard top-left
                for (int row = 0; row < fbH / 2; ++row)
                {
                    auto beg = slot->pixels.begin();
                    std::swap_ranges(
                        beg + row * fbW * 3,
                        beg + (row + 1) * fbW * 3,
                        beg + (fbH - 1 - row) * fbW * 3);
                }
                slot->width  = fbW;
                slot->height = fbH;

                // Upload preview texture while we still own the slot (before deliver)
                glBindTexture(GL_TEXTURE_2D, previewTex);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbW, fbH, 0,
                    GL_RGB, GL_UNSIGNED_BYTE, slot->pixels.data());
                previewReady = true;
                previewW     = fbW;
                previewH     = fbH;

                // Deliver to net thread; then signal the condvar to wake it
                g_frameSender.deliver();
                g_frameCv.notify_one();
            }
            else
            {
                log->warn("Frame pool exhausted — skipping capture this frame");
            }
        }

        glfwSwapBuffers(window);
    }

    // --- Shutdown ---------------------------------------------------------
    log->info("Shutting down");
    g_running.store(false);
    g_frameCv.notify_all();   // unblock net thread if it is waiting on the condvar
    netThr.join();

    glDeleteTextures(1, &previewTex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

    // Persist settings for next run
    SettingsRegistry::instance().saveJson("stream_server_settings.json");
    log->info("Settings saved to stream_server_settings.json. Exiting.");

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
