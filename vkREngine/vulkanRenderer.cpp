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
    
    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };
#ifndef NDEBUG
    if (!checkValidationLayerSupport(validationLayers)) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }
#endif
    
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (glfwExtensions == nullptr) {
        throw std::runtime_error("Failed to get Instance Extensions!");
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
        throw std::runtime_error("VkInstance does not support required extensions!");
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
#ifndef NDEBUG
        static_cast<uint32_t>(validationLayers.size()),     //enabledLayerCount
        validationLayers.data(),                            //ppEnabledLayerNames
#else
        0,
        nullptr,
#endif
        static_cast<uint32_t>(requiredExtensions.size()),   //enabledExtensionCount
        requiredExtensions.data()                           //ppEnabledExtensionNames
    };
    
    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance!");
        
    }
}

void VulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = getQueueFamilies(coreDevice.physicalDevice);
    
    float priority = 1.0f; // Vulkan needs to know how to handle multiple multiple queues. [1] is high priority.
    VkDeviceQueueCreateInfo queueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     //sType
        nullptr,                                        //pNext
        0,                                              //flags
        static_cast<uint32_t>(indices.graphicsFamily),  //queueFamilyIndex
        1,                                              //queueCount
        &priority                                       //pQueuePriorities
    };
    
    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,   //sType
        nullptr,                                //pNext
        0,                                      //flags
        1,                                      //queueCreateInfoCount
        &queueCreateInfo,                       //pQueueCreateInfos
        0,                                      //enabledLayerCount
        nullptr,                                //ppEnabledLayerNames
        0,                                      //enabledExtensionCount
        nullptr,                                //ppEnabledExtensionNames
        &physicalDeviceFeatures                 //pEnabledFeatures
    };
    
    if (vkCreateDevice(coreDevice.physicalDevice, &deviceCreateInfo, nullptr, &coreDevice.logicalDevice) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Logical Device!");
    }
    
    // Handle queues created
    vkGetDeviceQueue(coreDevice.logicalDevice, indices.graphicsFamily, 0, &graphicsQueue);
}

void VulkanRenderer::getPhysicalDevice() {
    // Get the number of physical device
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
    
    if (deviceCount == 0) {
        throw std::runtime_error("Cannot find GPU that supports Vulkan Instance!");
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

bool VulkanRenderer::checkInstanceExtensionSupport(const std::vector<const char *>& checkExtensions) {
    // Get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    
    // Get the list of avaliable extension
    std::vector<VkExtensionProperties> extentions{extensionCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extentions.data());
    
    for (const auto& checkExtension : checkExtensions) {
        bool hasExtension = false;
        for (const auto& extension : extentions) {
            if (std::strcmp(checkExtension, extension.extensionName) == 0) {
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

bool VulkanRenderer::checkValidationLayerSupport(const std::vector<const char *>& checkLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
    
    for (const auto& checkLayer : checkLayers) {
        bool hasLayer = false;
        for (const auto& layer : availableLayers) {
            if (std::strcmp(checkLayer, layer.layerName) == 0) {
                hasLayer = true;
                break;
            }
        }
        
        if (!hasLayer) {
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









