#pragma once

#include <string>
#include <GLFW/glfw3.h>
#include "IWindow.hpp"


class Window : public IWindow<GLFWwindow> {
public:
    Window() = default;
    
    ~Window() override;
    
    void swapBuffer() override;
    
    void updateFpsCounter(float dt);

private:
    void initImpl(const char* title, int width, int height, bool fullscreen) override;

    void clearImpl(float r, float g, float b, float a) override;

    std::string m_title;

    double m_previousSeconds{};
    double m_currentSeconds{};
    int m_frameCount{};
};
