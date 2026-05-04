#include "mylib/imguiscene.h"

#include "glutils.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

#include <cstdio>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

#include "settings/dirty_tracker.hpp"
#include "settings/settings_item.hpp"

struct ImGuiSettings : DirtyTracker
{
    std::string fontPath{"resources/font/ComicMonoNF-Regular.ttf"};
    float      fontSize{36.0f};
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(ImGuiSettings, fontPath, fontSize)
protected:
    void onChanged() override
    {
        std::cout << "ImGuiSettings changed: fontPath=" << fontPath << ", fontSize=" << fontSize << std::endl;
    }
};
SettingsItem<ImGuiSettings> g_ImGuiSettings("ImGuiSettings");

void ImGuiScene::initScene()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    std::string fontPath = g_ImGuiSettings->fontPath;
    ImFont* font = io.Fonts->AddFontFromFileTTF(fontPath.c_str(), g_ImGuiSettings->fontSize);

    if (font == nullptr)
    {
        std::cerr << "Failed to load font: " << fontPath << std::endl;
        std::exit(EXIT_FAILURE);
    }

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        style.WindowRounding              = 0.0f;
        style.Colors[ImGuiCol_WindowBg].w = 1.0f;
    }
    ImGui_ImplGlfw_InitForOpenGL(glfwGetCurrentContext(), true);
    ImGui_ImplOpenGL3_Init("#version 410");
}

void ImGuiScene::render()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Dockspace host window
    ImGuiIO&             io           = ImGui::GetIO();
    ImGuiWindowFlags     window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    const ImGuiViewport* viewport     = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);
    window_flags |=
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
    bool dockspaceOpen = true;
    // Make central node pass through so scene remains visible
    ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_PassthruCentralNode;
    window_flags |= ImGuiWindowFlags_NoBackground;
    ImGui::Begin("DockSpace", &dockspaceOpen, window_flags);
    ImGui::PopStyleVar(2);
    if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
    {
        ImGuiID dockspace_id = ImGui::GetID("MainDockSpace");
        ImGui::DockSpace(dockspace_id, ImVec2(0, 0), dock_flags);
    }
    ImGui::End();

    // Example control window
    ImGui::Begin("Triangle");
    ImGui::Text("Simple triangle scene");
    ImGui::Separator();
    static bool showDemo = false;
    ImGui::Checkbox("Show Demo", &showDemo);
    ImGui::Text("FPS: %.1f", io.Framerate);
    ImGui::End();
    if (showDemo)
        ImGui::ShowDemoWindow(&showDemo);

    renderImGuiWidgets();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
    {
        GLFWwindow* backup_current_context = glfwGetCurrentContext();
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
        glfwMakeContextCurrent(backup_current_context);
    }
}
