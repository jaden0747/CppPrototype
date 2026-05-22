// ---------------------------------------------------------------------------
// stream_client.cpp — Phase 1: raw RGB frame streaming client
//
// Connects to a stream_server over TCP, receives [width][height][len][RGB]
// frames, uploads them to an OpenGL texture, and displays them live inside
// an ImGui window using ImGui::Image.
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
#include <cstring>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <cstdio>

// ---------------------------------------------------------------------------
// Shared state between network thread and render thread
// ---------------------------------------------------------------------------
struct ReceivedFrame
{
    std::mutex           mtx;
    std::vector<uint8_t> data;
    int                  width  = 0;
    int                  height = 0;
    bool                 hasNew = false;
};

struct ClientStats
{
    std::mutex   mtx;
    uint64_t     bytesRecv  = 0;
    uint64_t     framesRecv = 0;
    float        fps        = 0.0f;
    bool         connected  = false;
    std::string  status     = "Disconnected";
};

static ReceivedFrame     g_frame;
static ClientStats       g_stats;
static std::atomic<bool> g_running{false};
static std::atomic<bool> g_doConnect{false};   // set by UI → network thread picks up

// Connection params written by UI before g_doConnect is set
static std::string g_serverIp   = "127.0.0.1";
static int         g_serverPort = 9999;

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

// ---------------------------------------------------------------------------
// Network thread: wait for connect request → receive frames → loop
// ---------------------------------------------------------------------------
static void netThread()
{
    while (g_running)
    {
        if (!g_doConnect.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            continue;
        }

        // Snapshot connection params (written before g_doConnect was set)
        std::string ip   = g_serverIp;
        int         port = g_serverPort;

        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status    = "Connecting to " + ip + ":" + std::to_string(port) + " …";
            g_stats.connected = false;
        }

        sock_t fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (fd < 0)
        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status = "socket() failed";
            g_doConnect.store(false);
            continue;
        }

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port   = htons(static_cast<uint16_t>(port));
        if (::inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) != 1)
        {
            close_sock(fd);
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status = "Invalid IP: " + ip;
            g_doConnect.store(false);
            continue;
        }

        if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0)
        {
            close_sock(fd);
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status    = "Connection refused";
            g_stats.connected = false;
            g_doConnect.store(false);
            continue;
        }

        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status    = "Connected";
            g_stats.connected = true;
        }
        std::printf("[net] Connected to %s:%d\n", ip.c_str(), port);

        using clock = std::chrono::steady_clock;
        auto lastTime = clock::now();

        while (g_running && g_doConnect.load())
        {
            // Read 12-byte header: [width 4B][height 4B][payload_size 4B]
            uint8_t hdr[12];
            if (!recv_all(fd, hdr, 12)) break;

            uint32_t w, h, len;
            std::memcpy(&w,   hdr + 0, 4); w   = ntohl(w);
            std::memcpy(&h,   hdr + 4, 4); h   = ntohl(h);
            std::memcpy(&len, hdr + 8, 4); len = ntohl(len);

            if (len == 0 || len > 64u * 1024u * 1024u)   // sanity: max 64 MB
                break;

            std::vector<uint8_t> pixels(len);
            if (!recv_all(fd, pixels.data(), len)) break;

            // FPS measurement
            auto  now = clock::now();
            float dt  = std::chrono::duration<float>(now - lastTime).count();
            lastTime  = now;

            // Hand frame to render thread
            {
                std::lock_guard<std::mutex> lk(g_frame.mtx);
                g_frame.data   = std::move(pixels);
                g_frame.width  = static_cast<int>(w);
                g_frame.height = static_cast<int>(h);
                g_frame.hasNew = true;
            }

            {
                std::lock_guard<std::mutex> lk(g_stats.mtx);
                g_stats.bytesRecv += 12 + len;
                g_stats.framesRecv++;
                if (dt > 0.0f)
                    g_stats.fps = g_stats.fps * 0.9f + (1.0f / dt) * 0.1f;
            }
        }

        close_sock(fd);
        std::printf("[net] Disconnected from %s:%d\n", ip.c_str(), port);
        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            g_stats.status    = "Disconnected";
            g_stats.connected = false;
            g_stats.fps       = 0.0f;
        }
        g_doConnect.store(false);
    }
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

    g_running.store(true);
    std::thread netThr(netThread);

    // UI state
    char ipBuf[256];
    std::strncpy(ipBuf, "127.0.0.1", sizeof(ipBuf));
    int portBuf = 9999;

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Upload any pending frame to the GL texture (must be on render thread)
        {
            std::lock_guard<std::mutex> lk(g_frame.mtx);
            if (g_frame.hasNew && !g_frame.data.empty())
            {
                glBindTexture(GL_TEXTURE_2D, streamTex);
                if (g_frame.width != texW || g_frame.height != texH)
                {
                    // Reallocate texture storage when resolution changes
                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                        g_frame.width, g_frame.height, 0,
                        GL_RGB, GL_UNSIGNED_BYTE, g_frame.data.data());
                    texW = g_frame.width;
                    texH = g_frame.height;
                }
                else
                {
                    // Same resolution — cheaper sub-update
                    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
                        g_frame.width, g_frame.height,
                        GL_RGB, GL_UNSIGNED_BYTE, g_frame.data.data());
                }
                g_frame.hasNew = false;
            }
        }

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

        bool        connected;
        std::string status;
        float       recvFps;
        uint64_t    framesRecv, bytesRecv;
        {
            std::lock_guard<std::mutex> lk(g_stats.mtx);
            connected  = g_stats.connected;
            status     = g_stats.status;
            recvFps    = g_stats.fps;
            framesRecv = g_stats.framesRecv;
            bytesRecv  = g_stats.bytesRecv;
        }

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
                g_doConnect.store(true);   // release to network thread
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
                g_doConnect.store(false);
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
    g_running.store(false);
    g_doConnect.store(false);
    netThr.join();

    glDeleteTextures(1, &streamTex);
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
