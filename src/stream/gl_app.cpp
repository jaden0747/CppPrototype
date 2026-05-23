#include "stream/gl_app.hpp"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <utility>

namespace stream
{

GlApp::GlApp(GlAppConfig config)
    : config_(std::move(config))
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

    window_ = glfwCreateWindow(config_.width, config_.height, config_.title.c_str(), nullptr, nullptr);
    if (!window_)
    {
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(window_);
    glfwSwapInterval(config_.vsync ? 1 : 0);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    if (config_.docking)
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    imgui_initialized_ = true;
}

GlApp::~GlApp()
{
    if (imgui_initialized_)
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    if (window_)
        glfwDestroyWindow(window_);
    glfwTerminate();
}

bool GlApp::valid() const
{
    return window_ != nullptr && imgui_initialized_;
}

GLFWwindow* GlApp::window() const
{
    return window_;
}

void GlApp::run(const GlAppCallbacks& callbacks)
{
    if (!valid())
        return;

    while (!glfwWindowShouldClose(window_))
    {
        glfwPollEvents();

        if (callbacks.before_imgui)
            callbacks.before_imgui();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (config_.docking)
            draw_dockspace();

        if (callbacks.on_imgui)
            callbacks.on_imgui();

        ImGui::Render();
        int framebuffer_width  = 0;
        int framebuffer_height = 0;
        glfwGetFramebufferSize(window_, &framebuffer_width, &framebuffer_height);
        glViewport(0, 0, framebuffer_width, framebuffer_height);
        glClearColor(config_.clear_color[0], config_.clear_color[1], config_.clear_color[2], config_.clear_color[3]);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (callbacks.after_render)
            callbacks.after_render(framebuffer_width, framebuffer_height);

        glfwSwapBuffers(window_);
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

} // namespace stream
