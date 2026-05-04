#include "mylib/mylib.hpp"
#include "mylib/customscene.h"
#include "scene.h"
#include "scenerunner.h"

SettingsItem<AppSettings> g_AppSettings("AppSettings");

void Main::run()
{
    // init coding

    auto& reg = SettingsRegistry::instance();
    reg.loadJson("settings.json");

    SceneRunner runner("OpenGL Cookbook", g_AppSettings->width, g_AppSettings->height);

    std::unique_ptr<Scene> scene;
    scene = std::unique_ptr<Scene>(new CustomScene());

    if (!runner.run(std::move(scene)))
    {
        std::cerr << "SceneRunner failed to run the scene." << std::endl;
    }

    return;
}
