// ---------------------------------------------------------------------------
// main.cpp — Minimal prototype demonstrating:
//   1. Settings loaded from JSON
//   2. Sender/Receiver ports (multithreaded data transfer)
//   3. ImGui debug window (OpenGL + GLFW)
// ---------------------------------------------------------------------------
#include "settings/settings_item.hpp"
#include "settings/examples/app_config.hpp"
#include "settings/examples/render_settings.hpp"
#include "mylib/data_container.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <GLFW/glfw3.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <thread>

// ---------------------------------------------------------------------------
// Settings items (registered at static-init time)
// ---------------------------------------------------------------------------
SettingsItem<AppConfig>      g_app("AppConfig");
SettingsItem<RenderSettings> g_render("RenderSettings");

// ---------------------------------------------------------------------------
// Data ports demo: a background thread sends a counter value to the main loop
// ---------------------------------------------------------------------------
struct CounterData
{
    int   value = 0;
    float timestamp = 0.0f;
};

static dc::Mempool<CounterData>    g_counterPool(4);
static dc::SenderPort<CounterData> g_counterSender;
static std::atomic<bool>           g_running{true};

void backgroundThread()
{
    g_counterSender.connectMempool(g_counterPool);
    int count = 0;
    while (g_running.load())
    {
        CounterData* slot = g_counterSender.reserve();
        if (slot)
        {
            slot->value = count++;
            slot->timestamp = static_cast<float>(
                std::chrono::duration<double>(
                    std::chrono::steady_clock::now().time_since_epoch())
                    .count());
            g_counterSender.deliver();
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// ---------------------------------------------------------------------------
// Main
// ---------------------------------------------------------------------------
int main()
{
    // Load settings
    SettingsRegistry::instance().loadJson("settings.json");

    // GLFW init
    if (!glfwInit())
        return 1;

#ifdef __APPLE__
    // macOS supports OpenGL up to 4.1; Core Profile requires 3.2+ and FORWARD_COMPAT
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#else
    // Windows, Linux, WSL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(
        g_app->windowWidth, g_app->windowHeight,
        g_app->appName.c_str(), nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    // ImGui init
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    ImGui::StyleColorsDark();
    {
        ImFont* font = io.Fonts->AddFontFromFileTTF(
            g_app->uiFontPath.c_str(), g_app->uiFontSize);
        if (!font)
        {
            // Font file not found — fall back to built-in default
            ImFontConfig cfg;
            cfg.SizePixels = g_app->uiFontSize;
            io.Fonts->AddFontDefault(&cfg);
        }
    }
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __APPLE__
    ImGui_ImplOpenGL3_Init("#version 150");  // GL 3.2 Core → GLSL 1.50
#else
    ImGui_ImplOpenGL3_Init("#version 130");  // GL 3.3 → GLSL 1.30 (Windows/Linux/WSL)
#endif

    // Receiver port (main thread reads from background sender)
    dc::ReceiverPort<CounterData> counterReceiver;
    counterReceiver.connect(g_counterSender);

    // Start background sender thread
    std::thread bgThread(backgroundThread);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Update receiver (promote pending → active)
        counterReceiver.update();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // --- Fullscreen DockSpace so panels can be docked anywhere ---
        {
            ImGuiViewport* vp = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(vp->WorkPos);
            ImGui::SetNextWindowSize(vp->WorkSize);
            ImGui::SetNextWindowViewport(vp->ID);
            ImGuiWindowFlags host_flags =
                ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoResize   | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoDocking;
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
            ImGui::Begin("##DockSpace", nullptr, host_flags);
            ImGui::PopStyleVar();
            ImGui::DockSpace(ImGui::GetID("MainDockSpace"),
                ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
            ImGui::End();
        }

        // --- Debug panel: Settings ---
        ImGui::Begin("Settings");
        ImGui::Text("App: %s", g_app->appName.c_str());
        ImGui::Text("Window: %dx%d", g_app->windowWidth, g_app->windowHeight);
        ImGui::Text("Target FPS: %.1f", g_app->targetFps);
        ImGui::Separator();
        ImGui::ColorEdit4("Clear Color", g_render->clearColor.data());
        ImGui::Checkbox("Wireframe", &g_render->wireframe);
        ImGui::Text("Shadow Quality: %s", g_render->shadowQuality.c_str());
        ImGui::Text("Max Lights: %d", g_render->maxLights);
        ImGui::End();

        // --- Debug panel: Data Ports ---
        ImGui::Begin("Data Ports");
        if (counterReceiver.hasNewData())
        {
            const CounterData* d = counterReceiver.getData();
            ImGui::Text("Counter: %d", d->value);
            ImGui::Text("Timestamp: %.3f", d->timestamp);
        }
        else if (counterReceiver.hasData())
        {
            const CounterData* d = counterReceiver.getData();
            ImGui::Text("Counter: %d (no new data this frame)", d->value);
        }
        else
        {
            ImGui::Text("Waiting for data...");
        }
        ImGui::End();

        // Render
        ImGui::Render();
        int displayW, displayH;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glViewport(0, 0, displayW, displayH);
        glClearColor(
            g_render->clearColor[0], g_render->clearColor[1],
            g_render->clearColor[2], g_render->clearColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        // Cleanup receiver flag at end of frame
        counterReceiver.cleanup();
    }

    // Shutdown
    g_running.store(false);
    bgThread.join();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    // Save settings on exit
    SettingsRegistry::instance().saveJson("settings.json");

    return 0;
}
