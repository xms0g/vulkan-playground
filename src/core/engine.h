#pragma once

#include <memory>

class VulkanRenderer;
class Window;
class Engine {
public:
    Engine();

    ~Engine();

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
