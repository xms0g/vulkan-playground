#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

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

    void createRenderPass();

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

    Window* mWindow{};

	vk::raii::Context context;
	vk::raii::Instance instance{nullptr};
};

