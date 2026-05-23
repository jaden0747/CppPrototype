#pragma once

#include <array>
#include <functional>
#include <string>

struct GLFWwindow;

namespace stream
{

struct GlAppConfig
{
    std::string          title   = "Stream App";
    int                  width   = 1280;
    int                  height  = 720;
    bool                 vsync   = true;
    bool                 docking = true;
    std::array<float, 4> clear_color{0.08f, 0.08f, 0.10f, 1.0f};
};

struct GlAppCallbacks
{
    std::function<void()>                                              before_imgui;
    std::function<void()>                                              on_imgui;
    std::function<void(int framebuffer_width, int framebuffer_height)> after_render;
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
    void        run(const GlAppCallbacks& callbacks);

private:
    void draw_dockspace();

    GlAppConfig config_;
    GLFWwindow* window_            = nullptr;
    bool        imgui_initialized_ = false;
};

} // namespace stream
