#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "swapchain.hpp"

class Window;

class Renderer {
public:
	Renderer();

	~Renderer();

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

	vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities);

	uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);


	bool checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions);

	bool checkDeviceExtensionSupport(VkPhysicalDevice device);

	bool checkValidationLayerSupport(const std::vector<const char*>& checkValidationLayers);

	VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);

	VkShaderModule createShaderModule(const std::vector<char>& code);

	Window* mWindow{nullptr};

	vk::raii::Context mContext;
	vk::raii::Instance mInstance{nullptr};
	vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
	vk::raii::Device mDevice{nullptr};
	vk::raii::Queue mGraphicsQueue{nullptr};
	vk::raii::SurfaceKHR mSurface{nullptr};
	vk::raii::SwapchainKHR mSwapChain{nullptr};
	vk::SurfaceFormatKHR mSwapChainSurfaceFormat;
	vk::Extent2D mSwapChainExtent;
	std::vector<vk::Image> mSwapChainImages;
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};
