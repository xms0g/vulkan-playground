#pragma once

#include <vulkan/vulkan.hpp>
#include <SDL2/SDL_vulkan.h>
#include <SDL2/SDL.h>
#include <stdexcept>
#include <vector>
#include <iostream>

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();
    
    int init(SDL_Window* window);
    
private:
    SDL_Window* m_window;
    
    // Vulkan components
    VkInstance m_instance;
    
    void createInstance();
};

