#ifndef CUSTOMSCENE_H
#define CUSTOMSCENE_H

#include "imguiscene.h"
#include "cookbookogl.h"

#include "mylib/ipc/fifo_channel.h"
#include "mylib/ipc/producer.h"
#include "mylib/ipc/consumer.h"

#include "settings/settings_item.hpp"

#include <deque>
#include <memory>
#include <string>

class CustomScene : public ImGuiScene
{
private:
    GLuint vaoHandle     = 0;
    GLuint programHandle = 0;

    static constexpr int MAX_MESSAGES = 200;

    FifoChannel                m_channel{"/tmp/customscene_fifo"};
    std::unique_ptr<Producer>  m_producer;
    std::unique_ptr<Consumer>  m_consumer;
    std::deque<std::string>    m_messages;   // only touched on the main thread

public:
    CustomScene();
    ~CustomScene() override;

    void initScene() override;
    void update(float t) override;
    void render() override;
    void resize(int w, int h) override;
    void renderImGuiWidgets() override;
};

#endif // CUSTOMSCENE_H
