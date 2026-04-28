#include "mylib/mylib.hpp"
#include "mylib/customscene.h"
#include "scene.h"
#include "scenerunner.h"

#define WIDTH 1920.0f
#define HEIGHT 1080.0f
#define SCALE 1.5f

void Main::run()
{
    int fbw = static_cast<int>(WIDTH * SCALE);
    int fbh = static_cast<int>(HEIGHT * SCALE);
    SceneRunner runner("OpenGL Cookbook", fbw, fbh);

    std::unique_ptr<Scene> scene;
    scene = std::unique_ptr<Scene>(new CustomScene());

    if (!runner.run(std::move(scene)))
    {
        std::cerr << "SceneRunner failed to run the scene." << std::endl;
    }

    return;
}

int Main::add(int a, int b)
{
    return a + b;
}
