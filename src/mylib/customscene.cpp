#include "mylib/customscene.h"

#include <cstdio>
#include <signal.h>
#include <string>
using std::string;

#include "glutils.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

CustomScene::CustomScene() = default;

CustomScene::~CustomScene()
{
    // Order matters:
    //   1. Close read end first — producer gets EPIPE on its next write and exits.
    //   2. Stop producer — stop() notifies the sleep cv; thread exits almost immediately.
    //   3. Destroy channel — unlinks the FIFO.
    m_consumer.reset();
    m_producer.reset();
    m_channel.destroy();
}

void CustomScene::initScene()
{
    ImGuiScene::initScene();

    // Ignore SIGPIPE process-wide so write() returns -1/EPIPE instead of
    // killing the process when the read end is closed.
    signal(SIGPIPE, SIG_IGN);

    m_channel.create();

    // Consumer opens first (non-blocking) so the producer's openWriter() returns
    // immediately rather than blocking indefinitely.
    m_consumer = std::make_unique<Consumer>(m_channel);
    m_consumer->open();

    m_producer = std::make_unique<Producer>(m_channel, Producer::sensorSource());
    m_producer->start();
}

void CustomScene::renderImGuiWidgets()
{
    for (auto& line : m_consumer->drain())
    {
        m_messages.push_back(std::move(line));
        if (static_cast<int>(m_messages.size()) > MAX_MESSAGES)
            m_messages.pop_front();
    }

    ImGui::Begin("FIFO Logger");
    ImGui::Text("Messages: %zu / %d", m_messages.size(), MAX_MESSAGES);
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
        m_messages.clear();
    ImGui::Separator();

    ImGui::BeginChild("log_scroll", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
    for (const auto& msg : m_messages)
        ImGui::TextUnformatted(msg.c_str());
    if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
        ImGui::SetScrollHereY(1.0f);
    ImGui::EndChild();

    ImGui::End();
}

void CustomScene::update(float t)
{
    (void)t;
}

void CustomScene::render()
{
    glClear(GL_COLOR_BUFFER_BIT);
    if (programHandle != 0)
    {
        glUseProgram(programHandle);
        glBindVertexArray(vaoHandle);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);
    }
    ImGuiScene::render();
}

void CustomScene::resize(int w, int h)
{
    width  = w;
    height = h;
    glViewport(0, 0, w, h);
}
