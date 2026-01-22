#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include "imguiscene.h"

#include "cookbookogl.h"
#include <string>
#include <memory>

class CustomScene : public ImGuiScene
{
private:
    GLuint vaoHandle;
    GLuint programHandle;

    void linkMe(GLint vertShader, GLint fragShader);
    void compileShaderProgram();

    std::string getShaderInfoLog(GLuint shader);
    std::string getProgramInfoLog(GLuint program);

public:
    CustomScene();

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int, int) override;
};

#endif // CUSTOMSCENE_H
