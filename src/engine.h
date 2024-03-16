#pragma once

#include <memory>
#include "vulkanRenderer.h"
#include "window.h"

class Engine {
public:
    Engine();

    void run();

private:
    void processInput();

    void update();

    void render();

    std::unique_ptr<VulkanRenderer> renderer;
    std::unique_ptr<Window> window;

    double deltaTime{};
    double millisecsPreviousFrame{0};
};
