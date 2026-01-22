#include "mylib/mylib.hpp"
#include "mylib/customscene.h"
#include "scene.h"
#include "scenerunner.h"

void Main::run()
{
    SceneRunner runner("OpenGL Cookbook", 1280, 720);

    std::unique_ptr<Scene> scene;
    scene = std::unique_ptr<Scene>(new CustomScene());

    if (!runner.run(std::move(scene)))
    {
        std::cerr << "SceneRunner failed to run the scene." << std::endl;
    }

    return;
}
