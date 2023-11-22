#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <set>
#include <iostream>
#include "window.h"
#include "utilities.hpp"
#include "vulkanValidation.hpp"

class VulkanRenderer {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer();
    
    int init(Window* window);
    
private:
    Window* m_window;
    
    // Vulkan components
    VkInstance m_instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } coreDevice;
    
    VkQueue m_graphicsQueue;
    VkQueue m_presentationQueue;
    VkSurfaceKHR m_surface;
    
    void createInstance();
    void createLogicalDevice();
    void createSurface();
    
    void getPhysicalDevice();
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
    SwapChainDetails getSwapChainDetails(VkPhysicalDevice device);
    
    // Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    bool checkValidationLayerSupport(const std::vector<const char*>& checkValidationLayers);
    bool checkDeviceSuitable(VkPhysicalDevice device);
};

