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
    
    int Init(SDL_Window* window);
    
private:
    SDL_Window* mWindow;
    
    // Vulkan components
    VkInstance instance;
    
    void CreateInstance();
};

