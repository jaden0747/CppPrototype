#include "stream/gl_app.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <chrono>
#include <utility>

namespace stream
{

// Store GlApp* in the GLFW user pointer so static callbacks can reach it.
static GlApp* get_app(GLFWwindow* w)
{
    return static_cast<GlApp*>(glfwGetWindowUserPointer(w));
}

GlApp::GlApp(GlAppConfig config)
    : m_config(std::move(config))
{
    if (!glfwInit())
        return;

#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    const char* glsl_version = "#version 150";
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    const char* glsl_version = "#version 130";
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(m_config.width, m_config.height, m_config.title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        glfwTerminate();
        return;
    }

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
        glfwTerminate();
        return;
    }

    glfwSwapInterval(m_config.vsync ? 1 : 0);

    // Install GLFW callbacks before ImGui so ImGui wraps them automatically.
    install_glfw_callbacks();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    if (m_config.docking)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!m_config.font_path.empty())
        io.Fonts->AddFontFromFileTTF(m_config.font_path.c_str(), m_config.font_size);
    else
        io.FontGlobalScale = m_config.font_size / 13.0f; // 13px is ImGui's built-in size

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    m_imgui_initialized = true;
}

GlApp::~GlApp()
{
    for (auto& layer : m_layers)
        layer->onDetach();

    if (m_imgui_initialized)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (m_window)
        glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool GlApp::valid() const
{
    return m_window != nullptr && m_imgui_initialized;
}

GLFWwindow* GlApp::window() const
{
    return m_window;
}

void GlApp::pushLayer(std::unique_ptr<GlLayer> layer)
{
    layer->onAttach(*this);
    m_layers.push_back(std::move(layer));
}

void GlApp::close()
{
    m_close = true;
}

void GlApp::run()
{
    if (!valid())
        return;

    using clock   = std::chrono::steady_clock;
    auto lastTime = clock::now();

    const double maxSleep = 1.0 / std::max(1.0f, m_config.max_render_fps);

    while (!glfwWindowShouldClose(m_window) && !m_close)
    {
        // Adaptive sleep: honour layer wakeup requests so capture-driven layers
        // can fire at exactly their target rate without spinning.
        auto   now      = std::chrono::steady_clock::now();
        double sleepSec = maxSleep;
        for (const auto& layer : m_layers)
        {
            auto wakeup = layer->nextWakeup();
            if (wakeup != std::chrono::steady_clock::time_point::max())
            {
                double secs = std::chrono::duration<double>(wakeup - now).count();
                sleepSec    = std::min(sleepSec, std::max(0.0, secs));
            }
        }
        glfwWaitEventsTimeout(sleepSec);

        auto  frameNow = clock::now();
        float dt       = std::chrono::duration<float>(frameNow - lastTime).count();
        lastTime       = frameNow;

        for (auto& layer : m_layers)
            layer->onUpdate(dt);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (m_config.docking)
            draw_dockspace();

        for (auto& layer : m_layers)
            layer->onImGui();

        ImGui::Render();

        int fbW = 0, fbH = 0;
        glfwGetFramebufferSize(m_window, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(
            m_config.clear_color[0], m_config.clear_color[1], m_config.clear_color[2], m_config.clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        for (auto& layer : m_layers)
            layer->onRender(fbW, fbH);

        glfwSwapBuffers(m_window);
    }
}

void GlApp::draw_dockspace()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("##DockSpace", nullptr, flags);
    ImGui::PopStyleVar();
    ImGui::DockSpace(ImGui::GetID("MainDS"), ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    ImGui::End();
}

void GlApp::install_glfw_callbacks()
{
    glfwSetKeyCallback(m_window, key_callback);
    glfwSetMouseButtonCallback(m_window, mouse_button_callback);
    glfwSetScrollCallback(m_window, scroll_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
}

void GlApp::key_callback(GLFWwindow* w, int key, int scancode, int action, int mods)
{
    // ImGui wraps this; forward to layers only when ImGui doesn't want keyboard.
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureKeyboard)
        return;
    GlApp* app = get_app(w);
    for (auto& layer : app->m_layers)
        if (layer->onKey(key, scancode, action, mods))
            break;
}

void GlApp::mouse_button_callback(GLFWwindow* w, int button, int action, int mods)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
    GlApp* app = get_app(w);
    for (auto& layer : app->m_layers)
        if (layer->onMouseButton(button, action, mods))
            break;
}

void GlApp::scroll_callback(GLFWwindow* w, double xoff, double yoff)
{
    const ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse)
        return;
    GlApp* app = get_app(w);
    for (auto& layer : app->m_layers)
        if (layer->onScroll(xoff, yoff))
            break;
}

void GlApp::framebuffer_size_callback(GLFWwindow* w, int width, int height)
{
    GlApp* app = get_app(w);
    for (auto& layer : app->m_layers)
        layer->onResize(width, height);
}

} // namespace stream
