#include "mylib/customscene.h"

#include <cstdio>
#include <cstdlib>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
using std::string;
#include <iterator>
#include <vector>

#include "glutils.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

CustomScene::CustomScene()
{
}

void CustomScene::initScene()
{
    ImGuiScene::initScene();
    compileShaderProgram();

    // clang-format off
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f,  0.5f, 0.0f
    };
    // clang-format on

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    GLuint vbo;
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
}

void CustomScene::compileShaderProgram()
{
    const char* vertSrc = R"GLSL(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )GLSL";

    const char* fragSrc = R"GLSL(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0);
        }
    )GLSL";

    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSrc, nullptr);
    glCompileShader(vertShader);
    GLint status = GL_FALSE;
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::string log = getShaderInfoLog(vertShader);
        printf("Vertex shader compile error:\n%s\n", log.c_str());
    }

    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSrc, nullptr);
    glCompileShader(fragShader);
    glGetShaderiv(fragShader, GL_COMPILE_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::string log = getShaderInfoLog(fragShader);
        printf("Fragment shader compile error:\n%s\n", log.c_str());
    }

    linkMe(vertShader, fragShader);

    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
}

void CustomScene::linkMe(GLint vertShader, GLint fragShader)
{
    programHandle = glCreateProgram();
    glAttachShader(programHandle, vertShader);
    glAttachShader(programHandle, fragShader);
    glLinkProgram(programHandle);

    GLint status = GL_FALSE;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if (status != GL_TRUE)
    {
        std::string log = getProgramInfoLog(programHandle);
        printf("Program link error:\n%s\n", log.c_str());
        glDeleteProgram(programHandle);
        programHandle = 0;
    }
}

std::string CustomScene::getShaderInfoLog(GLuint shader)
{
    GLint logLen;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);

    std::string log;
    if (logLen > 0)
    {
        log.resize(logLen, ' ');
        GLsizei written;
        glGetShaderInfoLog(shader, logLen, &written, &log[0]);
    }

    return log;
}

std::string CustomScene::getProgramInfoLog(GLuint program)
{
    GLint logLen;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLen);

    std::string log;
    if (logLen > 0)
    {
        log.resize(logLen, ' ');
        GLsizei written;
        glGetProgramInfoLog(program, logLen, &written, &log[0]);
    }
    return log;
}

void CustomScene::update(float t)
{
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

