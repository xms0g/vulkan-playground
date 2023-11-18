#include "vulkanRenderer.h"
#include <cstring>


VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(m_instance, nullptr);
}

int VulkanRenderer::init(GLFWwindow* window) {
    m_window = window;
    
    try {
        createInstance();
        getPhysicalDevice();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return 0;
}

void VulkanRenderer::getPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    std::vector<VkPhysicalDevice> physicalDevices{deviceCount};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
    
}

void VulkanRenderer::createInstance() {
    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "vkREngine",                // Custom name of the application
        VK_MAKE_VERSION(1, 0, 0),   // Custom version of the application
        "No Engine",                // Custom engine name
        VK_MAKE_VERSION(1, 0, 0),   // Custom engine version
        VK_API_VERSION_1_3          // The vulkan version
    };
    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (glfwExtensions == nullptr) {
        throw std::runtime_error("Failed to get Instance Extensions");
    }
    
    std::vector<const char*> requiredExtensions;
    for(uint32_t i = 0; i < glfwExtensionCount; i++) {
        requiredExtensions.emplace_back(glfwExtensions[i]);
    }
    
#ifdef __APPLE__
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    
    // Check instance extensions supported
    if (!checkInstanceExtensionSupport(requiredExtensions)) {
        throw std::runtime_error("VkInstance does not support required extensions");
    }
    
    VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             //sType
        nullptr,                                            //pNext
#ifdef __APPLE__
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,   //flags
#else
        0
#endif
        &appInfo,                                           //pApplicationInfo
        0,                                                  //enabledLayerCount
        nullptr,                                            //ppEnabledLayerNames
        static_cast<uint32_t>(requiredExtensions.size()),   //enabledExtensionCount
        requiredExtensions.data()                           //ppEnabledExtensionNames
    };
    
    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
        
    }
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char *>& checkExtensions) {
    // Get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    std::vector<VkExtensionProperties> extentions{extensionCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extentions.data());
    
    for (const auto& checkExtension : checkExtensions) {
        bool hasExtension = false;
        for (const auto& extension : extentions) {
            if (std::strcmp(checkExtension, extension.extensionName)) {
                hasExtension = true;
                break;
            }
        }
        
        if (!hasExtension) {
            return false;
        }
    }
    
    return true;
}




