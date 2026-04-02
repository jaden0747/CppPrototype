#include "scenerunner.h"
#include "glutils.h"
#include <iostream>

SceneRunner::SceneRunner(const std::string& windowTitle, int width, int height, int samples)
    : window(nullptr), fbw(0), fbh(0), debug(true) {
    if (!glfwInit()) exit(EXIT_FAILURE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#elif defined(__WSL__)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
#endif
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    if (debug) glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    if (samples > 0) glfwWindowHint(GLFW_SAMPLES, samples);

    window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Unable to create OpenGL context." << std::endl;
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &fbw, &fbh);
    if (!gladLoadGL()) exit(-1);
    GLUtils::dumpGLInfo();
    glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
#ifndef __APPLE__
    if (debug) {
        glDebugMessageCallback(GLUtils::debugCallback, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 0,
                             GL_DEBUG_SEVERITY_NOTIFICATION, -1, "Start debugging");
    }
#endif
}

SceneRunner::~SceneRunner() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

int SceneRunner::run(std::unique_ptr<Scene> scene) {
    mainLoop(window, std::move(scene));
#ifndef __APPLE__
    if (debug) {
        glDebugMessageInsert(GL_DEBUG_SOURCE_APPLICATION, GL_DEBUG_TYPE_MARKER, 1,
                             GL_DEBUG_SEVERITY_NOTIFICATION, -1, "End debug");
    }
#endif
    return EXIT_SUCCESS;
}

std::string SceneRunner::parseCLArgs(int argc, char** argv, std::map<std::string, std::string>& sceneData) {
    if (argc < 2) {
        printHelpInfo(argv[0], sceneData);
        exit(EXIT_FAILURE);
    }
    std::string recipeName = argv[1];
    auto it = sceneData.find(recipeName);
    if (it == sceneData.end()) {
        std::cerr << "Unknown recipe: " << recipeName << "\n";
        printHelpInfo(argv[0], sceneData);
        exit(EXIT_FAILURE);
    }
    return recipeName;
}

void SceneRunner::printHelpInfo(const char* exeFile, std::map<std::string, std::string>& sceneData) {
    std::cout << "Usage: " << exeFile << " recipe-name\n\n";
    std::cout << "Recipe names:\n";
    for (auto& it : sceneData) {
        std::cout << "  " << it.first << " : " << it.second << '\n';
    }
}

void SceneRunner::mainLoop(GLFWwindow* window, std::unique_ptr<Scene> scene) {
    scene->setDimensions(fbw, fbh);
    scene->initScene();
    scene->resize(fbw, fbh);
    while (!glfwWindowShouldClose(window) && !glfwGetKey(window, GLFW_KEY_ESCAPE)) {
        GLUtils::checkForOpenGLError(__FILE__, __LINE__);
        scene->update(float(glfwGetTime()));
        scene->render();
        glfwSwapBuffers(window);
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            scene->animate(!scene->animating());
        }
    }
}
