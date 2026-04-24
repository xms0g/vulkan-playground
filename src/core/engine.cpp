#include "engine.h"
#include "window.h"
#include "../rendering/renderer.h"

Engine::Engine()
	: mRenderer(std::make_unique<Renderer>()),
	  mWindow(std::make_unique<Window>()) {
	try {
		mWindow->init("Vulkan");
		mRenderer->init(mWindow.get());
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
}

Engine::~Engine() = default;

void Engine::run() {
	while (!mWindow->shouldClose()) {
		processInput();
		update();
		render();
	}
	mRenderer->waitIdle();
}

void Engine::processInput() {
	glfwPollEvents();
}

void Engine::update() {
	mDeltaTime = (glfwGetTime() - mMillisecsPreviousFrame) / 1000.0f;
	mMillisecsPreviousFrame = glfwGetTime();
	mWindow->updateFpsCounter(mDeltaTime);
}

void Engine::render() const {
	mRenderer->render();
}
