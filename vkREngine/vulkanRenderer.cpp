#include "vulkanRenderer.h"
#include <cstring>


VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyDevice(coreDevice.logicalDevice, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

int VulkanRenderer::init(GLFWwindow* window) {
    m_window = window;
    
    try {
        createInstance();
        getPhysicalDevice();
        createLogicalDevice();
    } catch (const std::runtime_error& e) {
        throw std::runtime_error(e.what());
    }
    return 0;
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

void VulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = getQueueFamilies(coreDevice.physicalDevice);
    
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        nullptr,
        0,
        static_cast<uint32_t>(indices.graphicsFamily),
        1,
        &priority
    };
    
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        nullptr,
        0,
        1,
        &queueCreateInfo,
        0,
        nullptr,
        0,
        nullptr,
        &physicalDeviceFeatures
    };
    
    if (vkCreateDevice(coreDevice.physicalDevice, &deviceCreateInfo, nullptr, &coreDevice.logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Logical Device");
    }
    
    // Handle queues created
    vkGetDeviceQueue(coreDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
}

void VulkanRenderer::getPhysicalDevice() {
    // Get the number of physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        throw std::runtime_error("Cannot find GPU that supports Vulkan instance");
    }
    
    // Get the list of the physical device
    std::vector<VkPhysicalDevice> physicalDevices{deviceCount};
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, physicalDevices.data());
    
    for (const auto&device : physicalDevices) {
        if (checkDeviceSuitable(device)) {
            coreDevice.physicalDevice = device;
            break;
        }
    }
}

bool VulkanRenderer::checkInstanceExtensionSupport(std::vector<const char *>& checkExtensions) {
    // Get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    // Get the list of avaliable extension
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

bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {
    // Information about the device(ID, name, type, vendor, etc)
//    VkPhysicalDeviceProperties deviceProperties;
//    vkGetPhysicalDeviceProperties(device, &deviceProperties);
//    
//    // Information about what the device can do(geo shader, tess shader, etc)
//    VkPhysicalDeviceFeatures deviceFeatures;
//    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
    
    QueueFamilyIndices indices = getQueueFamilies(device);
    
    return indices.isValid();
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) { 
    QueueFamilyIndices indices;
    
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    
    std::vector<VkQueueFamilyProperties> queueFamilies{queueFamilyCount};
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }
        
        if (indices.isValid())
            break;
        
        i++;
    }
    
    return indices;
}









