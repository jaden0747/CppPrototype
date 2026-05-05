#include "mylib/mylib.hpp"

#if !defined(_WIN32)
#include "mylib/customscene.h"
#include "scene.h"
#include "scenerunner.h"
#endif

SettingsItem<AppSettings> g_AppSettings("AppSettings");

void Main::run()
{
    // init coding

    auto& reg = SettingsRegistry::instance();
    reg.loadJson("settings.json");

#if defined(_WIN32)
    // The default scene (CustomScene) is a FIFO logger that depends on POSIX
    // IPC (mkfifo/SIGPIPE/O_NONBLOCK) and is therefore unavailable on Windows.
    // Build the project on Linux/macOS/WSL to run the IPC demo, or build the
    // settings_demo / cli_server targets on Windows instead.
    std::cerr << "Application: CustomScene requires POSIX IPC; not supported on Windows.\n"
              << "Try the settings_demo or cli_server target instead, "
              << "or build on Linux/WSL." << std::endl;
    return;
#else
    SceneRunner runner("OpenGL Cookbook", g_AppSettings->width, g_AppSettings->height);

    std::unique_ptr<Scene> scene;
    scene = std::unique_ptr<Scene>(new CustomScene());

    if (!runner.run(std::move(scene)))
    {
        std::cerr << "SceneRunner failed to run the scene." << std::endl;
    }

    return;
#endif
}
