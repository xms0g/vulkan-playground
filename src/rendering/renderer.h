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

	void render();

	void waitIdle() const;

private:
	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void createLogicalDevice();

	void createSwapchain();

	vk::raii::ImageView createImageView(const vk::Image& image, vk::Format format) const;

	void createImageViews();

	void recreateSwapchain();

	void createGraphicsPipeline();

	void createCommandPool();

	void createVertexBuffer(vk::DeviceSize size);

	void createIndexBuffer(vk::DeviceSize size);

	void createUniformBuffers();

	void createDescriptorPool();

	void createDescriptorSets();

	void createDescriptorSetLayout();

	void createBuffer(
		vk::DeviceSize size,
		vk::BufferUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::raii::Buffer& buffer,
		vk::raii::DeviceMemory& bufferMemory) const;

	void createCommandBuffers();

	void recordCommandBuffer(uint32_t imageIndex) const;

	void transitionImageLayout(
		uint32_t imageIndex,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask) const;

	void transitionImageLayout(const vk::raii::Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const;

	void createSyncObjects();

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
	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties) const;

	void copyBuffer(const vk::raii::Buffer& srcBuffer, const vk::raii::Buffer& dstBuffer, vk::DeviceSize size) const;

	void copyBufferToImage(const vk::raii::Buffer& buffer, const vk::raii::Image& image, uint32_t width, uint32_t height) const;

	void updateUniformBuffer(uint32_t currentImage) const;

	[[nodiscard]]
	vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const;

	void createTextureImage(const char* path);

	void createImage(
		uint32_t width,
		uint32_t height,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties,
		vk::raii::Image& image,
		vk::raii::DeviceMemory& imageMemory) const;

	void createTextureImageView();

	void createTextureSampler();

	vk::raii::CommandBuffer beginSingleTimeCommands() const;

	void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

	Window* mWindow{nullptr};

	vk::raii::Context mContext;
	vk::raii::Instance mInstance{nullptr};
	vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
	vk::raii::Device mDevice{nullptr};
	vk::raii::Queue mGraphicsQueue{nullptr};
	uint32_t mGraphicsQueueFamilyIndex{static_cast<uint32_t>(~0)};
	vk::raii::SurfaceKHR mSurface{nullptr};
	vk::raii::SwapchainKHR mSwapChain{nullptr};
	vk::SurfaceFormatKHR mSwapChainSurfaceFormat;
	vk::Extent2D mSwapChainExtent;
	std::vector<vk::Image> mSwapChainImages;
	std::vector<vk::raii::ImageView> mSwapChainImageViews;
	vk::raii::DescriptorSetLayout mDescriptorSetLayout{nullptr};
	vk::raii::DescriptorPool mDescriptorPool{nullptr};
	std::vector<vk::raii::DescriptorSet> mDescriptorSets;
	vk::raii::PipelineLayout mPipelineLayout{nullptr};
	vk::raii::Pipeline mGraphicsPipeline{nullptr};
	vk::raii::Buffer mVertexBuffer{nullptr};
	vk::raii::DeviceMemory mVertexBufferMemory{nullptr};
	vk::raii::Buffer mIndexBuffer{nullptr};
	vk::raii::DeviceMemory mIndexBufferMemory{nullptr};
	std::vector<vk::raii::Buffer> mUniformBuffers;
	std::vector<vk::raii::DeviceMemory> mUniformBuffersMemory;
	std::vector<void*> mUniformBuffersMapped;
	vk::raii::CommandPool mCommandPool{nullptr};
	std::vector<vk::raii::CommandBuffer> mCommandBuffers;
	std::vector<vk::raii::Semaphore> mPresentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> mRenderFinishedSemaphores;
	std::vector<vk::raii::Fence> mFences;
	uint32_t mFrameIndex{0};
	vk::raii::Image mTextureImage{nullptr};
	vk::raii::DeviceMemory mTextureImageMemory{nullptr};
	vk::raii::ImageView mTextureImageView{nullptr};
	vk::raii::Sampler mTextureSampler{nullptr};
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};
