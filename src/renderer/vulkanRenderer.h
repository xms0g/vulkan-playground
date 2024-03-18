#pragma once

#include "swapchain.hpp"
#include "queueFamily.hpp"

class Window;
class VulkanRenderer {
public:
    VulkanRenderer() = default;

    ~VulkanRenderer();

    int init(Window* window);

private:
    void createInstance();

    void createLogicalDevice();

    void createSurface();

    void createSwapchain();

    void createGraphicsPipeline();

    // Getters
    void getPhysicalDevice();

    QueueFamilyIndices getQueueFamilies(VkPhysicalDevice device);

    SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);

    // Support Functions
    bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);

    bool checkDeviceExtensionSupport(VkPhysicalDevice device);

    bool checkValidationLayerSupport(const std::vector<const char*>& checkValidationLayers);

    bool checkDeviceSuitable(VkPhysicalDevice device);

    VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

    VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& modes);

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

    VkShaderModule createShaderModule(const std::vector<char>& code);

    Window* m_window{};

    // Vulkan components
    VkInstance m_instance{};
    struct {
        VkPhysicalDevice physicalDevice{};
        VkDevice logicalDevice{};
    } coreDevice{};

    VkQueue m_graphicsQueue{};
    VkQueue m_presentationQueue{};
    VkSurfaceKHR m_surface{};
    VkSwapchainKHR m_swapchain{};
    std::vector<SwapchainImage> m_swapchainImages{};

    // Utility
    VkFormat swapchainImageFormat{};
    VkExtent2D swapchainExtent{};
};

