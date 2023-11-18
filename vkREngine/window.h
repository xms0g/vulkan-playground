#pragma once

#include <string>
#include <GLFW/glfw3.h>
#include "IWindow.hpp"


class Window : public IWindow<GLFWwindow> {
public:
    explicit Window(const char* title, int width = 800, int height = 600, bool fullscreen = false);

    ~Window() override;

    void clearImpl(float r, float g, float b, float a) override;
    
    void swapBuffer() override;
    
    void updateFpsCounter(float dt);

private:
    const std::string m_title;

    double m_previousSeconds{};
    double m_currentSeconds{};
    int m_frameCount{};
};
