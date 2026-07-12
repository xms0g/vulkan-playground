#include "device.h"
#include <set>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <chrono>
#include <random>
#include <unordered_set>
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "buffer.h"
#include "commandBuffer.h"
#include "swapchain.h"
#include "commandPool.h"
#include "descriptorPool.h"
#include "descriptorSet.h"
#include "descriptorSetLayout.h"
#include "deviceExtension.hpp"
#include "image.h"
#include "pipelineBuilder.h"
#include "sample.hpp"
#include "validation.hpp"
#include "mesh/vertex.hpp"
#include "image/stb_image.h"
#include "obj/tiny_obj_loader.h"
#include "../core/window.h"
#include "../config/config.hpp"
#include "../io/filesystem.h"

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;

Device::Device(Window& window) : mWindow(window) {
}

Device::~Device() = default;

void Device::init() {
	try {
		createInstance();
		setupDebugMessenger();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createDescriptorSetLayout();
		createPipelines();
		createCommandPool();
		createColorResources();
		createDepthResources();
		loadModel(fs::path(ASSET_DIR + MODEL_PATH).c_str());
		mVertexBuffer = createDeviceLocalBuffer(vertices.data(), sizeof(Vertex) * vertices.size(),
		                                        vk::BufferUsageFlagBits::eVertexBuffer);
		mIndexBuffer = createDeviceLocalBuffer(indices.data(), sizeof(uint32_t) * indices.size(),
		                                       vk::BufferUsageFlagBits::eIndexBuffer);
		createUniformBuffers();
		createTextureImage(fs::path(ASSET_DIR + TEXTURE_PATH).c_str());
		createTextureSampler();
		createDescriptorPool();
		createGraphicsDescriptorSets();
		createCommandBuffers();
		createSyncObjects();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
}

void Device::prepareFrame() {
	auto fenceResult = mDevice.waitForFences(*mFences[mFrameIndex], vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to wait for fence!");
	}

	mImageIndex = mSwapchain->acquireNextImage(mPresentCompleteSemaphores[mFrameIndex]);

	updateUniformBuffer(mFrameIndex);

	mDevice.resetFences(*mFences[mFrameIndex]);
}

void Device::submitGraphics() {
	recordGraphicsCommandBuffer(mImageIndex);

	vk::PipelineStageFlags waitDestinationStageMask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*mPresentCompleteSemaphores[mFrameIndex],
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &**mGraphicsCommandBuffers[mFrameIndex],
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*mRenderFinishedSemaphores[mImageIndex]
	};

	mQueue.submit(submitInfo, *mFences[mFrameIndex]);
}

void Device::presentFrame() {
	const vk::PresentInfoKHR presentInfoKHR{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*mRenderFinishedSemaphores[mImageIndex],
		.swapchainCount = 1,
		.pSwapchains = &***mSwapchain,
		.pImageIndices = &mImageIndex
	};

	auto result = mQueue.presentKHR(presentInfoKHR);
	if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || mWindow.windowResized()) {
		mWindow.windowResized(false);
		mSwapchain->recreate(mSurface, mDevice, mPhysicalDevice, *mWindow);
	} else {
		assert(result == vk::Result::eSuccess);
	}

	mFrameIndex = (mFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Device::waitIdle() const {
	mDevice.waitIdle();
}

void Device::getPhysicalDevice() {
	const auto physicalDevices = mInstance.enumeratePhysicalDevices();

	if (physicalDevices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	const auto deviceIt = std::ranges::find_if(
		physicalDevices, [&](const auto& phyDevice) { return checkDeviceSuitable(phyDevice); });

	if (deviceIt != physicalDevices.end()) {
		mPhysicalDevice = *deviceIt;
		mDepthFormat = findDepthFormat();
	}
}

void Device::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "Particle Simulation",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	if (enableValidationLayers) {
		std::unordered_set<std::string> supportedValidationLayers;
		for (const auto& layer: mContext.enumerateInstanceLayerProperties()) {
			supportedValidationLayers.insert(layer.layerName);
		}

		bool supportedRequiredValidationLayers = std::ranges::all_of(validationLayers, [&](const auto& layer) {
			return supportedValidationLayers.contains(layer);
		});

		if (!supportedRequiredValidationLayers) {
			throw std::runtime_error("Required Validation Layers not supported");
		}
	}

	const auto glfwExtensions = getRequiredInstanceExtensions();

	std::unordered_set<std::string> supportedExtensions;
	for (const auto& [extensionName, specVersion]: mContext.enumerateInstanceExtensionProperties()) {
		supportedExtensions.insert(extensionName);
	}

	std::vector<const char*> requiredExtensions;
	for (const auto& extension: glfwExtensions) {
		if (!supportedExtensions.contains(extension)) {
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(extension));
		}
		requiredExtensions.emplace_back(extension);
	}

	vk::InstanceCreateFlagBits flags{0};
#ifdef __APPLE__
	requiredExtensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);
	flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

	const vk::InstanceCreateInfo createInfo{
		.flags = flags,
		.pApplicationInfo = &appInfo,
		.enabledLayerCount = static_cast<uint32_t>(validationLayers.size()),
		.ppEnabledLayerNames = validationLayers.data(),
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	mInstance = vk::raii::Instance(mContext, createInfo);
}

void Device::setupDebugMessenger() {
	if constexpr (!enableValidationLayers) {
		return;
	}

	constexpr vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		vk::DebugUtilsMessageSeverityFlagBitsEXT::eError
	};

	constexpr vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
		vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
	};

	constexpr vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
		.messageSeverity = severityFlags,
		.messageType = messageTypeFlags,
		.pfnUserCallback = &debugCallback
	};

	mDebugMessenger = mInstance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}

void Device::createSurface() {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(*mInstance, &*mWindow, nullptr, &surface) != 0) {
		throw std::runtime_error("Failed to create window surface!");
	}

	mSurface = vk::raii::SurfaceKHR(mInstance, surface);
}

void Device::createLogicalDevice() {
	const std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
		    queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eCompute &&
		    mPhysicalDevice.getSurfaceSupportKHR(i, *mSurface)) {
			mQueueIndex = i;
			break;
		}
	}

	if (mQueueIndex == ~0) {
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// Create a chain of feature structures
	const vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
		vk::PhysicalDeviceTimelineSemaphoreFeaturesKHR> featureChain = {
		{.features = {.samplerAnisotropy = true}},
		{.synchronization2 = true, .dynamicRendering = true},
		{.extendedDynamicState = true},
		{.timelineSemaphore = true}
	};

	float queuePriority = 0.5f;
	const vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = mQueueIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	const vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	mDevice = vk::raii::Device(mPhysicalDevice, deviceCreateInfo);
	mQueue = vk::raii::Queue(mDevice, mQueueIndex, 0);
}

void Device::createSwapchain() {
	mSwapchain = std::make_unique<Swapchain>(mSurface, mDevice, mPhysicalDevice, *mWindow);
}

void Device::createDescriptorSetLayout() {
	mGraphicsDescriptorSetLayout = std::make_unique<DescriptorSetLayout>(mDevice);
	mGraphicsDescriptorSetLayout->addBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex)
			.addBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
			.build();
}

void Device::createPipelines() {
	PipelineBuilder builder{mDevice, mPhysicalDevice};
	Shader shader{mDevice, std::string(SHADER_BINARY_DIR) + SHADER_NAME};

	mGraphicsPipeline = std::make_unique<GraphicsPipeline>(
		builder,
		shader,
		*mGraphicsDescriptorSetLayout,
		1,
		mSwapchain->surfaceFormat(),
		mDepthFormat,
		Vertex::layout());
}

void Device::createCommandPool() {
	mCommandPool = std::make_unique<CommandPool>(
		mDevice,
		mQueueIndex,
		vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
}

void Device::createDescriptorPool() {
	mDescriptorPool = std::make_unique<DescriptorPool>(mDevice);
	mDescriptorPool->addMaxSets(MAX_FRAMES_IN_FLIGHT)
			.addPoolFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet)
			.addPoolSize(vk::DescriptorType::eUniformBuffer, MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(vk::DescriptorType::eCombinedImageSampler, MAX_FRAMES_IN_FLIGHT)
			.build();
}

void Device::loadModel(const char* path) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;

	if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path)) {
		throw std::runtime_error(warn + err);
	}

	for (const auto& shape: shapes) {
		for (const auto& index: shape.mesh.indices) {
			Vertex vertex{};
			vertex.pos = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			vertex.texCoord = {
				attrib.texcoords[2 * index.texcoord_index + 0],
				1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
			};

			vertex.color = {1.0f, 1.0f, 1.0f};
			vertices.push_back(vertex);
			indices.push_back(indices.size());
		}
	}
}

std::unique_ptr<Buffer> Device::createDeviceLocalBuffer(const void* data, vk::DeviceSize size,
                                                        const vk::BufferUsageFlags usage) {
	Buffer stagingBuffer{
		size,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	void* mem = stagingBuffer.map(size);
	memcpy(mem, data, size);
	stagingBuffer.unmap();

	auto buffer = std::make_unique<Buffer>(
		size,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferDst | usage,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	copyBuffer(*buffer, stagingBuffer, size);

	return buffer;
}

void Device::createUniformBuffers() {
	mUniformBuffers.clear();

	constexpr vk::DeviceSize bufferSize = sizeof(UniformBufferObject);
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mUniformBuffers.emplace_back(
			bufferSize,
			mDevice,
			mPhysicalDevice,
			vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		mUniformBuffers[i].map(bufferSize);
	}
}

void Device::createGraphicsDescriptorSets() {
	const DescriptorSetAllocator allocator(mDevice, *mDescriptorPool);
	mGraphicsDescriptorSets = allocator.allocate(MAX_FRAMES_IN_FLIGHT, **mGraphicsDescriptorSetLayout);

	DescriptorSetWriter writer(mDevice);
	writer.reserve(MAX_FRAMES_IN_FLIGHT * 2);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		writer.writeBuffer(
					*mGraphicsDescriptorSets[i],
					0,
					vk::DescriptorType::eUniformBuffer,
					**mUniformBuffers[i],
					0,
					sizeof(UniformBufferObject))
				.writeImage(
					*mGraphicsDescriptorSets[i],
					1,
					vk::DescriptorType::eCombinedImageSampler,
					mTextureSampler,
					mTextureImage->imageView(),
					vk::ImageLayout::eShaderReadOnlyOptimal);

		writer.update();
	}
}

void Device::createCommandBuffers() {
	mGraphicsCommandBuffers.clear();

	for (uint32_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mGraphicsCommandBuffers.emplace_back(mDevice, *mCommandPool, vk::CommandBufferLevel::ePrimary);
	}
}

void Device::createColorResources() {
	mColorImage = std::make_unique<Image>(
		mDevice,
		mPhysicalDevice,
		mSwapchain->extent().width,
		mSwapchain->extent().height,
		1,
		Sample::getMaxUsableSampleCount(mPhysicalDevice),
		mSwapchain->surfaceFormat().format,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransientAttachment | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	mColorImage->createImageView(mDevice, vk::ImageAspectFlagBits::eColor);
}

void Device::createDepthResources() {
	mDepthImage = std::make_unique<Image>(
		mDevice,
		mPhysicalDevice,
		mSwapchain->extent().width,
		mSwapchain->extent().height,
		1,
		Sample::getMaxUsableSampleCount(mPhysicalDevice),
		mDepthFormat,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	mDepthImage->createImageView(mDevice, vk::ImageAspectFlagBits::eDepth);
}

void Device::createTextureImage(const char* path) {
	int texWidth, texHeight, texChannels;
	void* pixels = stbi_load(path, &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	const vk::DeviceSize imageSize = texWidth * texHeight * 4;
	const uint32_t mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

	if (!pixels) {
		throw std::runtime_error("Failed to load texture image!");
	}

	Buffer stagingBuffer{
		imageSize,
		mDevice,
		mPhysicalDevice,
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent
	};

	void* mem = stagingBuffer.map(imageSize);
	memcpy(mem, pixels, imageSize);
	stagingBuffer.unmap();

	stbi_image_free(pixels);

	mTextureImage = std::make_unique<Image>(
		mDevice,
		mPhysicalDevice,
		texWidth,
		texHeight,
		mipLevels,
		vk::SampleCountFlagBits::e1,
		vk::Format::eR8G8B8A8Srgb,
		vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal
	);

	const vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();

	Image::transitionImageLayout(
		***mTextureImage,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eTransferDstOptimal,
		vk::AccessFlagBits2::eNone,
		vk::AccessFlagBits2::eTransferWrite,
		vk::PipelineStageFlagBits2::eTopOfPipe,
		vk::PipelineStageFlagBits2::eTransfer,
		vk::ImageAspectFlagBits::eColor,
		commandBuffer,
		mipLevels);

	copyBufferToImage(stagingBuffer, *mTextureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight),
	                  commandBuffer);
	mTextureImage->generateMipmaps(mPhysicalDevice, commandBuffer);

	mTextureImage->createImageView(mDevice, vk::ImageAspectFlagBits::eColor);
	endSingleTimeCommands(commandBuffer);
}

void Device::createTextureSampler() {
	const vk::PhysicalDeviceProperties properties = mPhysicalDevice.getProperties();
	const vk::SamplerCreateInfo samplerInfo{
		.magFilter = vk::Filter::eLinear,
		.minFilter = vk::Filter::eLinear,
		.mipmapMode = vk::SamplerMipmapMode::eLinear,
		.addressModeU = vk::SamplerAddressMode::eRepeat,
		.addressModeV = vk::SamplerAddressMode::eRepeat,
		.addressModeW = vk::SamplerAddressMode::eRepeat,
		.mipLodBias = 0.0f,
		.anisotropyEnable = vk::True,
		.maxAnisotropy = properties.limits.maxSamplerAnisotropy,
		.compareEnable = vk::False,
		.compareOp = vk::CompareOp::eAlways,
		.minLod = 0.0f,
		.maxLod = vk::LodClampNone
	};

	mTextureSampler = vk::raii::Sampler(mDevice, samplerInfo);
}

void Device::recordGraphicsCommandBuffer(const uint32_t imageIndex) {
	auto& commandBuffer = mGraphicsCommandBuffers[mFrameIndex];
	(*commandBuffer).reset();
	(*commandBuffer).begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	Image::transitionImageLayout(
		mSwapchain->image(imageIndex),
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{}, // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // dstStage
		vk::ImageAspectFlagBits::eColor,
		*commandBuffer,
		1
	);

	Image::transitionImageLayout(
		***mColorImage,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::AccessFlagBits2::eColorAttachmentWrite,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::PipelineStageFlagBits2::eColorAttachmentOutput,
		vk::ImageAspectFlagBits::eColor,
		*commandBuffer,
		1);
	// Transition depth image to depth attachment optimal layout
	Image::transitionImageLayout(
		***mDepthImage,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eDepthAttachmentOptimal,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::PipelineStageFlagBits2::eEarlyFragmentTests | vk::PipelineStageFlagBits2::eLateFragmentTests,
		vk::ImageAspectFlagBits::eDepth,
		*commandBuffer,
		1
	);

	constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
	constexpr vk::ClearValue clearDepth = vk::ClearDepthStencilValue(1.0f, 0);

	vk::RenderingAttachmentInfo colorAttachment = {
		.imageView = mColorImage->imageView(),
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.resolveMode = vk::ResolveModeFlagBits::eAverage,
		.resolveImageView = mSwapchain->imageView(imageIndex),
		.resolveImageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	vk::RenderingAttachmentInfo depthAttachmentInfo = {
		.imageView = mDepthImage->imageView(),
		.imageLayout = vk::ImageLayout::eDepthAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eDontCare,
		.clearValue = clearDepth
	};

	const vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = mSwapchain->extent()},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &colorAttachment,
		.pDepthAttachment = &depthAttachmentInfo
	};

	(*commandBuffer).beginRendering(renderingInfo);
	(*commandBuffer).bindPipeline(vk::PipelineBindPoint::eGraphics, **mGraphicsPipeline);
	(*commandBuffer).setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(mSwapchain->extent().width),
	                                             static_cast<float>(mSwapchain->extent().height), 0.0f, 1.0f));
	(*commandBuffer).setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapchain->extent()));
	(*commandBuffer).bindVertexBuffers(0, {***mVertexBuffer}, {0});
	(*commandBuffer).bindIndexBuffer(***mIndexBuffer, 0, vk::IndexTypeValue<decltype(indices)::value_type>::value);
	(*commandBuffer).bindDescriptorSets(
		vk::PipelineBindPoint::eGraphics,
		mGraphicsPipeline->layout(),
		0,
		*mGraphicsDescriptorSets[mFrameIndex],
		nullptr);
	(*commandBuffer).drawIndexed(indices.size(), 1, 0, 0, 0);
	(*commandBuffer).endRendering();

	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	Image::transitionImageLayout(
		mSwapchain->image(imageIndex),
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
		{}, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe, // dstStage
		vk::ImageAspectFlagBits::eColor,
		*commandBuffer,
		1
	);
	(*commandBuffer).end();
}

void Device::createSyncObjects() {
	assert(
		mPresentCompleteSemaphores.empty() &&
		mRenderFinishedSemaphores.empty() &&
		mFences.empty());

	for (size_t i = 0; i < mSwapchain->imageCount(); ++i) {
		mRenderFinishedSemaphores.emplace_back(mDevice, vk::SemaphoreCreateInfo());
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
		mPresentCompleteSemaphores.emplace_back(mDevice, vk::SemaphoreCreateInfo());
		mFences.emplace_back(mDevice, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
	}
}

void Device::updateUniformBuffer(uint32_t currentImage) const {
	static auto startTime = std::chrono::high_resolution_clock::now();

	const auto currentTime = std::chrono::high_resolution_clock::now();
	const float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	UniformBufferObject ubo{};
	ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.view = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	ubo.proj = glm::perspective(
		glm::radians(45.0f),
		static_cast<float>(mSwapchain->extent().width) / static_cast<float>(mSwapchain->extent().height),
		0.1f,
		10.0f);
	ubo.proj[1][1] *= -1;

	memcpy(mUniformBuffers[currentImage].mappedMemory(), &ubo, sizeof(ubo));
}

std::vector<const char*> Device::getRequiredInstanceExtensions() {
	uint32_t glfwExtensionCount = 0;
	const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

vk::Bool32 Device::debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	const vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

	return vk::False;
}

bool Device::checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice) {
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = phyDevice.getProperties().apiVersion >= vk::ApiVersion13;

	// Check if any of the queue families support graphics operations
	bool supportsGraphics = std::ranges::any_of(phyDevice.getQueueFamilyProperties(), [&](const auto& qfp) {
		return static_cast<bool>(qfp.queueFlags & vk::QueueFlagBits::eGraphics);
	});

	// Check if all required physicalDevice extensions are available
	std::unordered_set<std::string_view> availableSet;
	for (const auto& [extensionName, specVersion]: phyDevice.enumerateDeviceExtensionProperties()) {
		availableSet.insert(extensionName);
	}

	bool supportsAllRequiredExtensions = std::ranges::all_of(deviceExtensions, [&](const char* required) {
		return availableSet.contains(required);
	});

	// Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
	auto features2 = phyDevice.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan11Features,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	// Perform the checks with clear boolean logic
	bool supportsSamplerAnisotropy = features2.get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy;
	bool supportsShaderDrawParameters = features2.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters;
	bool supportsDynamicRendering = features2.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering;
	bool supportsSynchronization2 = features2.get<vk::PhysicalDeviceVulkan13Features>().synchronization2;
	bool supportsExtendedDynamicState = features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().
			extendedDynamicState;
	bool supportsRequiredFeatures =
			supportsSamplerAnisotropy &&
			supportsShaderDrawParameters &&
			supportsDynamicRendering &&
			supportsSynchronization2 &&
			supportsExtendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

vk::Format Device::findDepthFormat() const {
	vk::Format candidates[] = {vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint};

	return findSupportedFormat(
		candidates,
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Device::findSupportedFormat(
	const std::span<vk::Format> candidates,
	const vk::ImageTiling tiling,
	const vk::FormatFeatureFlags features) const {
	for (const auto& format: candidates) {
		const vk::FormatProperties props = mPhysicalDevice.getFormatProperties(format);

		if ((tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) ||
		    (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)) {
			return format;
		}
	}

	throw std::runtime_error("Failed to find supported depth format!");
}

void Device::copyBuffer(const Buffer& dstBuffer, const Buffer& srcBuffer, const vk::DeviceSize size) const {
	const vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();
	commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy(0, 0, size));
	endSingleTimeCommands(commandCopyBuffer);
}

void Device::copyBufferToImage(
	const Buffer& buffer,
	const Image& image,
	const uint32_t width,
	const uint32_t height,
	const vk::raii::CommandBuffer& commandBuffer) {
	vk::BufferImageCopy region{
		.bufferOffset = 0,
		.bufferRowLength = 0,
		.bufferImageHeight = 0,
		.imageSubresource = {vk::ImageAspectFlagBits::eColor, 0, 0, 1},
		.imageOffset = {0, 0, 0},
		.imageExtent = {width, height, 1}
	};

	commandBuffer.copyBufferToImage(**buffer, **image, vk::ImageLayout::eTransferDstOptimal, {region});
}

vk::raii::CommandBuffer Device::beginSingleTimeCommands() const {
	const vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = **mCommandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	vk::raii::CommandBuffer commandBuffer = std::move(mDevice.allocateCommandBuffers(allocInfo).front());
	constexpr vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
	commandBuffer.begin(beginInfo);

	return commandBuffer;
}

void Device::endSingleTimeCommands(const vk::raii::CommandBuffer& commandBuffer) const {
	commandBuffer.end();

	const vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};

	mQueue.submit(submitInfo, nullptr);
	mQueue.waitIdle();
}
