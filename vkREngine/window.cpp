#include "window.h"
#include <iostream>


Window::Window(const char* title, int width, int height, bool fullscreen) : m_title(title) {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Error initializing SDL" << std::endl;
        return;
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);
}

Window::~Window() {
    glfwDestroyWindow(m_window);
    glfwTerminate();
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

        glfwSetWindowTitle(m_window, tmp);
        m_frameCount = 0;
    }
    m_frameCount++;
}

void Window::swapBuffer() {
    glfwSwapBuffers(m_window);
}

void Window::clearImpl(float r, float g, float b, float a) {

}
