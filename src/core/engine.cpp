#include "engine.h"
#include "window.h"
#include "../renderer/vulkanRenderer.h"

Engine::Engine() :
        window(std::make_unique<Window>()),
        renderer(std::make_unique<VulkanRenderer>()) {
    try {
        window->init("vkREngine");
        renderer->init(window.get());
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(e.what());
    }
}

Engine::~Engine() = default;

void Engine::run() {
    while (!window->shouldClose()) {
        processInput();
        update();
        render();
    }
}

void Engine::processInput() {
    glfwPollEvents();
}

void Engine::update() {
    deltaTime = (glfwGetTime() - millisecsPreviousFrame) / 1000.0f;
    millisecsPreviousFrame = glfwGetTime();
    //window.updateFpsCounter(deltaTime);
}

void Engine::render() {
    window->swapBuffer();
}
