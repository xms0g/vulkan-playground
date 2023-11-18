#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <iostream>
#include "utilities.hpp"

class VulkanRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer();
    
    int init(GLFWwindow* window);
    
private:
    GLFWwindow* m_window;
    
    // Vulkan components
    VkInstance m_instance;
    struct {
        VkPhysicalDevice physicalDevice;
        VkDevice logicalDevice;
    } coreDevice;
    
    VkQueue graphicsQueue;
    
    void createInstance();
    void createLogicalDevice();
    
    void getPhysicalDevice();
    
    // Support Functions
    bool checkInstanceExtensionSupport(std::vector<const char*>& checkExtensions);
    bool checkDeviceSuitable(VkPhysicalDevice device);
    
    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);
};

