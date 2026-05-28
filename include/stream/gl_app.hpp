#pragma once

#include "stream/gl_layer.hpp"

#include <nlohmann/json.hpp>

#include <array>
#include <memory>
#include <string>
#include <vector>

struct GLFWwindow;

namespace stream
{

struct GlAppConfig
{
    std::string          title          = "Stream App";
    int                  width          = 1280;
    int                  height         = 720;
    bool                 vsync          = true;
    bool                 docking        = true;
    std::array<float, 4> clear_color    = {0.08f, 0.08f, 0.10f, 1.0f};
    std::string          font_path      = ""; // empty = ImGui built-in default
    float                font_size      = 16.0f;
    float                max_render_fps = 60.0f; // render loop cap; use WaitEventsTimeout to sleep

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(
        GlAppConfig,
        title,
        width,
        height,
        vsync,
        docking,
        clear_color,
        font_path,
        font_size,
        max_render_fps)
};

class GlApp
{
public:
    explicit GlApp(GlAppConfig config);
    ~GlApp();

    GlApp(const GlApp&)            = delete;
    GlApp& operator=(const GlApp&) = delete;

    bool        valid() const;
    GLFWwindow* window() const;

    void pushLayer(std::unique_ptr<GlLayer> layer);
    void close();
    void run();

private:
    void draw_dockspace();
    void install_glfw_callbacks();

    static void key_callback(GLFWwindow* w, int key, int scancode, int action, int mods);
    static void mouse_button_callback(GLFWwindow* w, int button, int action, int mods);
    static void scroll_callback(GLFWwindow* w, double xoff, double yoff);
    static void framebuffer_size_callback(GLFWwindow* w, int width, int height);

    GlAppConfig                           m_config;
    GLFWwindow*                           m_window            = nullptr;
    bool                                  m_imgui_initialized = false;
    bool                                  m_close             = false;
    std::vector<std::unique_ptr<GlLayer>> m_layers;
};

} // namespace stream
