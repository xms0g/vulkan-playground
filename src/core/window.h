#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN  
#include <GLFW/glfw3.h>
#include "IWindow.hpp"

class Window : public IWindow<GLFWwindow> {
public:
	Window() = default;

	~Window() override;

	void updateFpsCounter(double dt);

	[[nodiscard]]
	bool shouldClose() const;

	[[nodiscard]]
	bool windowResized() const;

	void windowResized(bool resized);

	void swapBuffer() override;

protected:
	void initImpl(const char* title, int width, int height, bool fullscreen) override;

	void clearImpl(float r, float g, float b, float a) override;

	std::string m_title;

	double m_previousSeconds{};
	double m_currentSeconds{};
	int m_frameCount{};

private:
	static void framebufferResizeCallback(GLFWwindow *window, int width, int height);

	bool mWindowResized{false};
};
