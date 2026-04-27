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
	const double currentTime = glfwGetTime();
	mDeltaTime = static_cast<float>(currentTime - mSecondsPreviousFrame);
	mSecondsPreviousFrame = currentTime;

	mWindow->updateFpsCounter(mDeltaTime);
}

void Engine::render() const {
	mRenderer->render();
}
