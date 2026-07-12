#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include <glm/glm.hpp>

#include "buffer.h"
#include "commandPool.h"
#include "image.h"
#include "pipelineBuilder.h"

class DescriptorSetLayout;
class CommandBuffer;
class DescriptorPool;
class Swapchain;
class Window;

class Device {
public:
	explicit Device(Window& window);

	~Device();

	void init();

	void prepareFrame();

	void submitGraphics();

	void presentFrame();

	void waitIdle() const;

private:
	// Getters
	void getPhysicalDevice();

	void createInstance();

	void setupDebugMessenger();

	void createSurface();

	void createLogicalDevice();

	void createSwapchain();

	void createDescriptorSetLayout();

	void createPipelines();

	void createDescriptorPool();

	void loadModel(const char* path);

	Buffer createDeviceLocalBuffer(const void* data, vk::DeviceSize size, vk::BufferUsageFlags usage);

	void createUniformBuffers();

	void createGraphicsDescriptorSets();

	void createCommandBuffers();

	void createColorResources();

	void createDepthResources();

	void createTextureImage(const char* path);

	void createTextureSampler();

	void recordGraphicsCommandBuffer(uint32_t imageIndex);

	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage) const;

	// Support Functions
	static std::vector<const char*> getRequiredInstanceExtensions();

	static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
		vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
		vk::DebugUtilsMessageTypeFlagsEXT type,
		const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData);

	static bool checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice);

	vk::Format findDepthFormat() const;

	vk::Format findSupportedFormat(
		std::span<vk::Format> candidates,
		vk::ImageTiling tiling,
		vk::FormatFeatureFlags features) const;

	vk::SampleCountFlagBits getMaxUsableSampleCount() const;

	void copyBuffer(const Buffer& dstBuffer, const Buffer& srcBuffer, vk::DeviceSize size) const;

	static void copyBufferToImage(
		const Buffer& buffer,
		const Image& image,
		uint32_t width,
		uint32_t height,
		const vk::raii::CommandBuffer& commandBuffer);

	[[nodiscard]]
	vk::raii::CommandBuffer beginSingleTimeCommands() const;

	void endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const;

	struct ComputePushConstants {
		float deltaTime{1.0f};
	};

	struct UniformBufferObject {
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	Window& mWindow;
	uint32_t mImageIndex{0};
	uint32_t mFrameIndex{0};
	vk::raii::Context mContext;
	vk::raii::Instance mInstance{nullptr};
	vk::raii::PhysicalDevice mPhysicalDevice{nullptr};
	vk::raii::Device mDevice{nullptr};
	vk::raii::Queue mQueue{nullptr};
	uint32_t mQueueIndex{static_cast<uint32_t>(~0)};
	vk::raii::SurfaceKHR mSurface{nullptr};
	std::unique_ptr<Swapchain> mSwapchain;
	std::unique_ptr<DescriptorSetLayout> mGraphicsDescriptorSetLayout;
	std::unique_ptr<DescriptorPool> mDescriptorPool;
	std::vector<vk::raii::DescriptorSet> mGraphicsDescriptorSets;
	GraphicsPipeline mGraphicsPipeline{};
	std::vector<Buffer> mUniformBuffers;
	Buffer mVertexBuffer{};
	Buffer mIndexBuffer{};
	CommandPool mCommandPool{};
	std::vector<CommandBuffer> mGraphicsCommandBuffers;
	std::vector<vk::raii::Semaphore> mPresentCompleteSemaphores;
	std::vector<vk::raii::Semaphore> mRenderFinishedSemaphores;
	std::vector<vk::raii::Fence> mFences;
	Image mColorImage{};
	Image mDepthImage{};
	vk::Format mDepthFormat{vk::Format::eUndefined};
	Image mTextureImage{};
	vk::raii::Sampler mTextureSampler{nullptr};
	vk::SampleCountFlagBits mMSAACount{vk::SampleCountFlagBits::e1};
	vk::raii::DebugUtilsMessengerEXT mDebugMessenger{nullptr};
};
