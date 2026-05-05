#ifndef IMGUI_SCENE_H
#define IMGUI_SCENE_H

#include "cookbookogl.h"
#include "scene.h"

class ImGuiScene : public Scene
{
public:
    ImGuiScene()           = default;
    ~ImGuiScene() override = default;

    void initScene() override;
    // void update(float t) override;
    void render() override;
    // void resize(int, int) override;

    // Called inside the ImGui frame, before Render(). Override in subclasses to add widgets.
    virtual void renderImGuiWidgets()
    {
    }
};

#endif // IMGUI_SCENE_H