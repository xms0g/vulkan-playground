#include "vulkanRenderer.h"
#include <cstring>
#include <set>
#include <algorithm>
#include <stdexcept>
#include "window.h"
#include "deviceExtension.hpp"
#include "vulkanValidation.hpp"
#include "../libs/filesystem/filesystem.h"


VulkanRenderer::~VulkanRenderer() {
    for (auto image: m_swapchainImages) {
        vkDestroyImageView(coreDevice.logicalDevice, image.imageView, nullptr);
    }
    vkDestroySwapchainKHR(coreDevice.logicalDevice, m_swapchain, nullptr);
    vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
    vkDestroyDevice(coreDevice.logicalDevice, nullptr);
    vkDestroyInstance(m_instance, nullptr);
}

int VulkanRenderer::init(Window* window) {
    m_window = window;

    try {
        createInstance();
        createSurface();
        getPhysicalDevice();
        createLogicalDevice();
        createSwapchain();
        createGraphicsPipeline();

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
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
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

    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance!");

    }
}

void VulkanRenderer::createLogicalDevice() {
    QueueFamilyIndices indices = getQueueFamilies(coreDevice.physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<int> queueFamilyIndices = {indices.graphicsFamily, indices.presentationFamily};

    // Queues the logical device needs to create
    for (const auto& queueFamilyIndex: queueFamilyIndices) {
        float priority = 1.0f; // Vulkan needs to know how to handle multiple queues. [1] is high priority.
        VkDeviceQueueCreateInfo queueCreateInfo = {
                VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,     //sType
                nullptr,                                        //pNext
                0,                                              //flags
                static_cast<uint32_t>(queueFamilyIndex),        //queueFamilyIndex
                1,                                              //queueCount
                &priority                                       //pQueuePriorities
        };

        queueCreateInfos.push_back(queueCreateInfo);
    }


    VkPhysicalDeviceFeatures physicalDeviceFeatures = {};
    VkDeviceCreateInfo deviceCreateInfo = {
            VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,               //sType
            nullptr,                                            //pNext
            0,                                                  //flags
            static_cast<uint32_t>(queueFamilyIndices.size()),   //queueCreateInfoCount
            queueCreateInfos.data(),                            //pQueueCreateInfos
            0,                                                  //enabledLayerCount
            nullptr,                                            //ppEnabledLayerNames
            static_cast<uint32_t>(deviceExtensions.size()),     //enabledExtensionCount
            deviceExtensions.data(),                            //ppEnabledExtensionNames
            &physicalDeviceFeatures                             //pEnabledFeatures
    };

    if (vkCreateDevice(coreDevice.physicalDevice, &deviceCreateInfo, nullptr, &coreDevice.logicalDevice) !=
        VK_SUCCESS) {
        throw std::runtime_error("Failed to create a Logical Device!");
    }

    // Handle queues created
    vkGetDeviceQueue(coreDevice.logicalDevice, indices.graphicsFamily, 0, &m_graphicsQueue);
    vkGetDeviceQueue(coreDevice.logicalDevice, indices.presentationFamily, 0, &m_presentationQueue);

}

void VulkanRenderer::createSurface() {
    if (glfwCreateWindowSurface(m_instance, m_window->nativeHandle(), nullptr, &m_surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a surface!");
    }
}

void VulkanRenderer::createSwapchain() {
    SwapchainDetails swapChainDetails = getSwapChainDetails(coreDevice.physicalDevice);

    // Find optimal surface values for our swap chain
    VkSurfaceFormatKHR surfaceFormat = chooseBestSurfaceFormat(swapChainDetails.surfaceFormats);
    VkPresentModeKHR presentationMode = chooseBestPresentationMode(swapChainDetails.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainDetails.surfaceCapabilities);

    // Get 1 more than the min to allow triple buffering
    uint32_t imageCount = swapChainDetails.surfaceCapabilities.minImageCount + 1;

    if (swapChainDetails.surfaceCapabilities.maxImageCount > 0 &&
        swapChainDetails.surfaceCapabilities.maxImageCount < imageCount) {
        imageCount = swapChainDetails.surfaceCapabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR swapChainCreateInfo = {};
    swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapChainCreateInfo.surface = m_surface;                                                    // Swapchain surface
    swapChainCreateInfo.imageFormat = surfaceFormat.format;                                     // Swapchain format
    swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;                             // Swapchain color space
    swapChainCreateInfo.presentMode = presentationMode;                                         // Swapchain presentation mode
    swapChainCreateInfo.imageExtent = extent;                                                   // Swapchain image extent
    swapChainCreateInfo.minImageCount = imageCount;                                             // Minumum images in swapchain
    swapChainCreateInfo.imageArrayLayers = 1;                                                   // Numbers of layers for each image in chain
    swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;                       // What attackemnt images will be used as
    swapChainCreateInfo.preTransform = swapChainDetails.surfaceCapabilities.currentTransform;   // Transform to perform on swapchain images
    swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;                     // How to handle blending images with external graphics(e.g other windows)
    swapChainCreateInfo.clipped = VK_TRUE;                                                      // Clip parts of the image not in window

    //Get Queue families
    QueueFamilyIndices indices = getQueueFamilies(coreDevice.physicalDevice);

    // If graphics and presentation families are different, then swapchain must let images be shared between families
    if (indices.graphicsFamily != indices.presentationFamily) {

        uint32_t queueFamilyIndices[2] = {
                static_cast<uint32_t>(indices.graphicsFamily),
                static_cast<uint32_t>(indices.presentationFamily)};

        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;      // Image Share handling
        swapChainCreateInfo.queueFamilyIndexCount = 2;                          // Number of queues to share images between
        swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;           // Array of queues to share between
    } else {
        swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapChainCreateInfo.queueFamilyIndexCount = 0;
        swapChainCreateInfo.pQueueFamilyIndices = nullptr;
    }
    // If old swap chain has been destroyed and this one replaces it, then link old one to quickly hand over responsibilities
    swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(coreDevice.logicalDevice, &swapChainCreateInfo, nullptr, &m_swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a swapchain!");
    }
    // Store for later
    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;

    // Get swapchain Imgaes
    uint32_t swapchainImageCount = 0;
    vkGetSwapchainImagesKHR(coreDevice.logicalDevice, m_swapchain, &swapchainImageCount, nullptr);

    std::vector<VkImage> swapchainImages{swapchainImageCount};
    vkGetSwapchainImagesKHR(coreDevice.logicalDevice, m_swapchain, &swapchainImageCount, swapchainImages.data());

    for (VkImage image: swapchainImages) {
        SwapchainImage swImage = {};
        swImage.image = image;
        swImage.imageView = createImageView(image, swapchainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);

        m_swapchainImages.push_back(swImage);
    }
}


void VulkanRenderer::createGraphicsPipeline() {
    // Read in SPIR-V code of shaders
    auto vertexShaderCode = Filesystem::readFile(Filesystem::path("shaders/vert.spv"));
    auto fragmentShaderCode = Filesystem::readFile(Filesystem::path("shaders/frag.spv"));

    // Create shader modules
    VkShaderModule vertexShaderModule = createShaderModule(vertexShaderCode);
    VkShaderModule fragmentShaderModule = createShaderModule(fragmentShaderCode);

    // Vertex Stage Creation Information
    VkPipelineShaderStageCreateInfo vertexShaderCreateInfo = {};
    vertexShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;      // Shader stage name
    vertexShaderCreateInfo.module = vertexShaderModule;             // Shader module to be used
    vertexShaderCreateInfo.pName = "main";                          // Entry point in the shader

    // Fragment Stage Creation Information
    VkPipelineShaderStageCreateInfo fragmenShaderCreateInfo = {};
    fragmenShaderCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmenShaderCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;    // Shader stage name
    fragmenShaderCreateInfo.module = fragmentShaderModule;           // Shader module to be used
    fragmenShaderCreateInfo.pName = "main";                          // Entry point in the shader

    // Put shader stage creation info into array
    VkPipelineShaderStageCreateInfo shaderStages[2] = {vertexShaderCreateInfo, fragmenShaderCreateInfo};

    // Vertex Input TODO:Put in vertex descriptions when resources created
    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
    vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputStateCreateInfo.vertexBindingDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexBindingDescriptions = nullptr; // List of vertex bindings descriptions
    vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 0;
    vertexInputStateCreateInfo.pVertexAttributeDescriptions = nullptr; // List of vertex attribute descriptions

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
    inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // Primitive type to assemble vertices as
    inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
    
    // Destroy shader modules, no longer needed
    vkDestroyShaderModule(coreDevice.logicalDevice, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(coreDevice.logicalDevice, vertexShaderModule, nullptr);


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

    for (const auto& device: physicalDevices) {
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
    for (const auto& queueFamily: queueFamilies) {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            indices.graphicsFamily = i;
        }

        VkBool32 presentationSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentationSupport);
        if (queueFamily.queueCount > 0 && presentationSupport) {
            indices.presentationFamily = i;
        }

        if (indices.isValid())
            break;

        i++;
    }

    return indices;
}

SwapchainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
    SwapchainDetails swapChainDetails;
    // Get the surface capabilities for the given surface on the given physical device
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &swapChainDetails.surfaceCapabilities);

    // Formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);
    if (formatCount != 0) {
        swapChainDetails.surfaceFormats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, swapChainDetails.surfaceFormats.data());

    }

    // Presentation
    uint32_t presentationCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationCount, nullptr);
    if (presentationCount != 0) {
        swapChainDetails.presentModes.resize(presentationCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentationCount,
                                                  swapChainDetails.presentModes.data());
    }

    return swapChainDetails;
}

bool VulkanRenderer::checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions) {
    // Get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    // Get the list of avaliable extension
    std::vector<VkExtensionProperties> extentions{extensionCount};
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extentions.data());

    for (const auto& checkExtension: checkExtensions) {
        bool hasExtension = false;
        for (const auto& extension: extentions) {
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


bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    // Get the number of extensions
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    if (extensionCount == 0) {
        return false;
    }

    std::vector<VkExtensionProperties> extensions{extensionCount};
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, extensions.data());

    for (const auto& deviceExtension: deviceExtensions) {
        bool hasExtension = false;
        for (const auto& extension: extensions) {
            if (std::strcmp(deviceExtension, extension.extensionName) == 0) {
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

bool VulkanRenderer::checkValidationLayerSupport(const std::vector<const char*>& checkLayers) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& checkLayer: checkLayers) {
        bool hasLayer = false;
        for (const auto& layer: availableLayers) {
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

    bool extensionSupported = checkDeviceExtensionSupport(device);

    bool swapChainValid = false;
    if (extensionSupported) {
        SwapchainDetails details = getSwapChainDetails(device);
        swapChainValid = !details.surfaceFormats.empty() && !details.presentModes.empty();
    }

    return indices.isValid() && extensionSupported && swapChainValid;
}

VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    if (formats.size() == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
        return {VK_FORMAT_R8G8B8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
    }

    for (const auto& format: formats) {
        if ((format.format == VK_FORMAT_R8G8B8_UNORM || format.format == VK_FORMAT_B8G8R8_UNORM) &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& modes) {
    for (const auto& mode: modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return mode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
    if (surfaceCapabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return surfaceCapabilities.currentExtent;
    }

    int width, height;
    glfwGetFramebufferSize(m_window->nativeHandle(), &width, &height);

    VkExtent2D newExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    newExtent.width = std::max(surfaceCapabilities.minImageExtent.width,
                               std::min(surfaceCapabilities.maxImageExtent.width, newExtent.width));
    newExtent.height = std::max(surfaceCapabilities.minImageExtent.height,
                                std::min(surfaceCapabilities.maxImageExtent.height, newExtent.height));

    return newExtent;
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo viewCreateInfo = {};
    viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image = image;                                       // Image to create view for
    viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;                    // Type of image
    viewCreateInfo.format = format;                                     // Format of image data
    viewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;        // Allows remapping of rgba components to other rgba values
    viewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    // Subresources allow the view to view only a part of image
    viewCreateInfo.subresourceRange.aspectMask = aspectFlags;           // Which aspect of image to view(e.g COLOR_BIT )
    viewCreateInfo.subresourceRange.baseMipLevel = 0;                   // Start mipmap level to view from
    viewCreateInfo.subresourceRange.levelCount = 1;                     // Number of mip level to view
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;                 // Start array level to view from
    viewCreateInfo.subresourceRange.layerCount = 1;                     // Number of array levels to view

    VkImageView imageView;
    if (vkCreateImageView(coreDevice.logicalDevice, &viewCreateInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create an Image View!");
    }

    return imageView;
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.codeSize = code.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(coreDevice.logicalDevice, &shaderModuleCreateInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create a shader module!");
    }

    return shaderModule;
}
















