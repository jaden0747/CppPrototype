// ---------------------------------------------------------------------------
// stream_server.cpp — Phase 1: raw RGB frame streaming server
//
// Renders an ImGui window, captures its own OpenGL framebuffer with
// glReadPixels after each frame, and streams the pixels to a connected
// client over TCP using a simple [width][height][len][pixels] protocol.
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
#include <cstdio>

static constexpr uint16_t SERVER_PORT = 9999;

// ---------------------------------------------------------------------------
// Shared state between render thread and network thread
// ---------------------------------------------------------------------------
struct SharedFrame
{
    std::mutex              mtx;
    std::condition_variable cv;
    std::vector<uint8_t>    data;   // RGB pixels, top-left origin
    int                     width  = 0;
    int                     height = 0;
    bool                    hasNew = false;
};

struct NetStats
{
    std::mutex   mtx;
    uint64_t     bytesSent  = 0;
    uint64_t     framesSent = 0;
    bool         connected  = false;
    std::string  clientAddr;
};

static SharedFrame       g_frame;
static NetStats          g_stats;
static std::atomic<bool> g_running{false};

// ---------------------------------------------------------------------------
// Helpers: send exactly n bytes (handles partial sends)
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

// Frame wire format: [4B width BE][4B height BE][4B payload_size BE][pixels RGB]
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

// ---------------------------------------------------------------------------
// Network thread: listen → accept → stream → loop
// ---------------------------------------------------------------------------
static void netThread()
{
    sock_t srv = ::socket(AF_INET, SOCK_STREAM, 0);
    if (srv < 0) { std::fprintf(stderr, "[net] socket() failed\n"); return; }

    int opt = 1;
    ::setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(SERVER_PORT);
    if (::bind(srv, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
    {
        std::fprintf(stderr, "[net] bind() failed on port %d\n", SERVER_PORT);
        close_sock(srv);
        return;
    }
    ::listen(srv, 1);
    std::printf("[net] Listening on TCP :%d\n", SERVER_PORT);

    while (g_running)
    {
        // Non-blocking accept via select (1 s timeout so we can check g_running)
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(srv, &fds);
        timeval tv{1, 0};
        int sel = ::select(static_cast<int>(srv) + 1, &fds, nullptr, nullptr, &tv);
        if (sel <= 0) continue;

        sockaddr_in caddr{};
        socklen_t   clen = sizeof(caddr);
        sock_t client = ::accept(srv, reinterpret_cast<sockaddr*>(&caddr), &clen);
        if (client < 0) continue;

        char clientIp[INET_ADDRSTRLEN] = {};
        ::inet_ntop(AF_INET, &caddr.sin_addr, clientIp, sizeof(clientIp));
        std::string addrStr = std::string(clientIp) + ":" + std::to_string(ntohs(caddr.sin_port));
        std::printf("[net] Client connected: %s\n", addrStr.c_str());

        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.connected  = true;
            g_stats.clientAddr = addrStr;
        }

        while (g_running)
        {
            // Wait for a new frame (200 ms timeout lets us re-check g_running)
            std::vector<uint8_t> pixelsCopy;
            int w = 0, h = 0;
            {
                std::unique_lock<std::mutex> lk(g_frame.mtx);
                bool got = g_frame.cv.wait_for(lk, std::chrono::milliseconds(200),
                    [&]{ return g_frame.hasNew || !g_running.load(); });
                if (!g_running) break;
                if (!got || !g_frame.hasNew) continue;
                pixelsCopy = g_frame.data;   // copy while holding lock
                w = g_frame.width;
                h = g_frame.height;
                g_frame.hasNew = false;
            }

            if (!send_frame(client, pixelsCopy.data(), w, h)) break;

            {
                std::lock_guard<std::mutex> lk(g_stats.mtx);
                g_stats.bytesSent  += 12 + static_cast<uint64_t>(w * h * 3);
                g_stats.framesSent++;
            }
        }

        close_sock(client);
        std::printf("[net] Client disconnected: %s\n", addrStr.c_str());
        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.connected  = false;
            g_stats.clientAddr.clear();
        }
    }

    close_sock(srv);
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
    ::signal(SIGPIPE, SIG_IGN);   // don't die on broken connection
#endif

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

    // Texture that shows the captured frame in the "Captured Preview" panel
    GLuint previewTex = 0;
    glGenTextures(1, &previewTex);
    glBindTexture(GL_TEXTURE_2D, previewTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    bool previewReady = false;

    g_running.store(true);
    std::thread netThr(netThread);

    // FPS tracking
    using clock = std::chrono::steady_clock;
    auto  lastFrameTime = clock::now();
    float renderFps     = 0.0f;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Render FPS
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
        ImGui::Text("Render FPS: %.1f", renderFps);
        ImGui::Separator();

        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            if (g_stats.connected)
            {
                ImGui::TextColored(ImVec4(0.2f, 1.0f, 0.2f, 1.0f),
                    "Connected: %s", g_stats.clientAddr.c_str());
                ImGui::Text("Frames sent : %llu",
                    static_cast<unsigned long long>(g_stats.framesSent));
                ImGui::Text("Data sent   : %.2f MB",
                    static_cast<double>(g_stats.bytesSent) / 1e6);
            }
            else
            {
                ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.1f, 1.0f),
                    "Waiting for client on :%d …", SERVER_PORT);
            }
        }

        ImGui::Separator();
        ImGui::TextDisabled("Protocol: [4B width][4B height][4B len][RGB]");
        ImGui::TextDisabled("Port: %d  |  Format: RGB 24-bit", SERVER_PORT);
        ImGui::End();

        // --- Captured Preview panel ---------------------------------------
        ImGui::Begin("Captured Preview");
        ImGui::TextDisabled("What the server is streaming right now:");
        if (previewReady)
        {
            ImVec2 avail  = ImGui::GetContentRegionAvail();
            float  aspect = 16.0f / 9.0f;
            {
                std::lock_guard<std::mutex> lk(g_frame.mtx);
                if (g_frame.height > 0)
                    aspect = static_cast<float>(g_frame.width) / static_cast<float>(g_frame.height);
            }
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
            std::vector<uint8_t> pixels(static_cast<size_t>(fbW * fbH * 3));
            glReadPixels(0, 0, fbW, fbH, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());

            // OpenGL origin is bottom-left; flip to top-left
            for (int row = 0; row < fbH / 2; ++row)
            {
                auto beg = pixels.begin();
                std::swap_ranges(
                    beg + row * fbW * 3,
                    beg + (row + 1) * fbW * 3,
                    beg + (fbH - 1 - row) * fbW * 3);
            }

            // Update preview texture (GL calls must be on render thread)
            glBindTexture(GL_TEXTURE_2D, previewTex);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fbW, fbH, 0,
                GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
            previewReady = true;

            // Hand latest frame to network thread
            {
                std::lock_guard<std::mutex> lk(g_frame.mtx);
                g_frame.data   = std::move(pixels);
                g_frame.width  = fbW;
                g_frame.height = fbH;
                g_frame.hasNew = true;
            }
            g_frame.cv.notify_one();
        }

        glfwSwapBuffers(window);
    }

    // --- Shutdown ---------------------------------------------------------
    g_running.store(false);
    g_frame.cv.notify_all();
    netThr.join();

    glDeleteTextures(1, &previewTex);
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();

#ifdef _WIN32
    WSACleanup();
#endif
    return 0;
}
