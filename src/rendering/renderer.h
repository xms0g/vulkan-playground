#pragma once
#include <cstdint>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

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

	void createImageViews();

	void createGraphicsPipeline();

	// Getters
	void getPhysicalDevice();

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

	[[nodiscard]]
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

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
	std::vector<vk::raii::ImageView> mSwapChainImageViews;
	vk::raii::PipelineLayout mPipelineLayout{nullptr};
	vk::raii::Pipeline mGraphicsPipeline{nullptr};
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};
