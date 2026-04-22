#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "swapchain.hpp"

class Window;

class VulkanRenderer {
public:
	VulkanRenderer() = default;

	~VulkanRenderer();

	int init(Window* window);

private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void createLogicalDevice();

	void createSwapchain();

	void createRenderPass();

	void createGraphicsPipeline();

	// Getters
	void getPhysicalDevice();

	SwapchainDetails getSwapChainDetails(VkPhysicalDevice device);

	// Support Functions
	std::vector<const char*> getRequiredInstanceExtensions();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	bool checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice);

	bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool checkValidationLayerSupport(const std::vector<const char*>& checkValidationLayers);

	VkSurfaceFormatKHR chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);

	VkPresentModeKHR chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& modes);

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkShaderModule createShaderModule(const std::vector<char>& code);

	Window* mWindow{};

	vk::raii::Context mContext;
	vk::raii::Instance mInstance{nullptr};
	vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
	vk::raii::Device mDevice{nullptr};
	vk::raii::Queue mGraphicsQueue{nullptr};
	vk::raii::SurfaceKHR mSurface{nullptr};
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};
