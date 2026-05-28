#pragma once

#include "mylib/imgui_log_sink.hpp"
#include "stream/gl_layer.hpp"

#include <memory>

namespace stream
{

// Reusable layer that renders a spdlog ImGui sink as a "Log" panel.
// Construct it, register the sink with Log::addSink(), then pushLayer() it.
class ImGuiLogLayer : public GlLayer
{
public:
    explicit ImGuiLogLayer(std::shared_ptr<ImGuiLogSink_mt> sink, const char* title = "Log")
        : m_sink(std::move(sink))
        , m_title(title)
    {
    }

    void onImGui() override
    {
        m_sink->draw(m_title);
    }

private:
    std::shared_ptr<ImGuiLogSink_mt> m_sink;
    const char*                      m_title;
};

} // namespace stream
