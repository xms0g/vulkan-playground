#include "window.h"

Window::~Window() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

bool Window::shouldClose() const {
	return glfwWindowShouldClose(mWindow);
}

void Window::swapBuffer() {
	glfwSwapBuffers(mWindow);
}

void Window::initImpl(const char* title, int width, int height, bool fullscreen) {
    m_title = title;

    if (glfwInit() != GLFW_TRUE) {
        throw std::runtime_error("Failed to initialize Window");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

void Window::clearImpl(float r, float g, float b, float a) {

}



void Window::updateFpsCounter(float dt) {
    double elapsedSeconds;

    m_currentSeconds += dt;
    elapsedSeconds = m_currentSeconds - m_previousSeconds;
    /* limit text updates to 4 per second */
    if (elapsedSeconds > 0.25) {
        m_previousSeconds = m_currentSeconds;
        char tmp[128];
        double fps = (double) m_frameCount / elapsedSeconds;

        snprintf(tmp, 128, "%s @ fps: %.2f", m_title.c_str(), fps);

        glfwSetWindowTitle(mWindow, tmp);
        m_frameCount = 0;
    }
    m_frameCount++;
}



