#pragma once

#include "cookbookogl.h"
#include "scene.h"
#include <GLFW/glfw3.h>

#include <map>
#include <memory>
#include <string>

#define WIN_WIDTH 800
#define WIN_HEIGHT 600

class SceneRunner
{
private:
    GLFWwindow* window;
    int         fbw, fbh;
    bool        debug;

public:
    SceneRunner(const std::string& windowTitle, int width = WIN_WIDTH, int height = WIN_HEIGHT, int samples = 0);
    ~SceneRunner();

    int run(std::unique_ptr<Scene> scene);

    static std::string parseCLArgs(int argc, char** argv, std::map<std::string, std::string>& sceneData);

private:
    static void printHelpInfo(const char* exeFile, std::map<std::string, std::string>& sceneData);
    void        mainLoop(GLFWwindow* window, std::unique_ptr<Scene> scene);
};
