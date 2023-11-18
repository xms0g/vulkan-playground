#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <vector>
#include <iostream>

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
    } MainDevice;
    
    void createInstance();
    void getPhysicalDevice();
    bool checkInstanceExtensionSupport(std::vector<const char*>& checkExtensions);
};

