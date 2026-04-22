#pragma once

#include <memory>

class Renderer;
class Window;
class Engine {
public:
    Engine();

    ~Engine();

    void run();

private:
    void processInput();

    void update();

    void render() const;

    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Window> window;

    double deltaTime{};
    double millisecsPreviousFrame{0};
};
