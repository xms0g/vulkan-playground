#include "renderer.h"
#include <cstring>
#include <set>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <unordered_set>
#include <GLFW/glfw3.h>
#include "deviceExtension.hpp"
#include "validation.hpp"
#include "../core/window.h"
#include "../config/config.hpp"
#include "../io/filesystem.h"

Renderer::Renderer() = default;

Renderer::~Renderer() = default;

int Renderer::init(Window* window) {
	mWindow = window;

	try {
		createInstance();
		setupDebugMessenger();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffer();
		createSyncObjects();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
	return 0;
}

void Renderer::render() {
	auto fenceResult = mDevice.waitForFences(*mDrawFence, vk::True, UINT64_MAX);
	if (fenceResult != vk::Result::eSuccess) {
		throw std::runtime_error("Failed to wait for fence!");
	}
	mDevice.resetFences(*mDrawFence);

	auto [result, imageIndex] = mSwapChain.acquireNextImage(UINT64_MAX, *mPresentCompleteSemaphore, nullptr);

	recordCommandBuffer(imageIndex);

	mGraphicsQueue.waitIdle();

	vk::PipelineStageFlags waitDestinationStageMask{vk::PipelineStageFlagBits::eColorAttachmentOutput};
	const vk::SubmitInfo submitInfo{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*mPresentCompleteSemaphore,
		.pWaitDstStageMask = &waitDestinationStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &*mCommandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &*mRenderFinishedSemaphore
	};

	mGraphicsQueue.submit(submitInfo, *mDrawFence);

	const vk::PresentInfoKHR presentInfoKHR{
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &*mRenderFinishedSemaphore,
		.swapchainCount = 1,
		.pSwapchains = &*mSwapChain,
		.pImageIndices = &imageIndex
	};

	result = mGraphicsQueue.presentKHR(presentInfoKHR);
	switch (result) {
		case vk::Result::eSuccess:
			break;
		case vk::Result::eSuboptimalKHR:
			std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
			break;
		default:
			break; // an unexpected result is returned!
	}
}

void Renderer::waitIdle() const {
	mDevice.waitIdle();
}

void Renderer::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "Hello Triangle",
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

		for (auto& layer: validationLayers) {
			if (!supportedValidationLayers.contains(layer)) {
				throw std::runtime_error("Required Validation Layer not supported: " + std::string(layer));
			}
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

void Renderer::setupDebugMessenger() {
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

void Renderer::createLogicalDevice() {
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = mPhysicalDevice.getQueueFamilyProperties();

	for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i) {
		if (queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics &&
		    mPhysicalDevice.getSurfaceSupportKHR(i, *mSurface)) {
			mGraphicsQueueFamilyIndex = i;
			break;
		}
	}

	if (mGraphicsQueueFamilyIndex == ~0) {
		throw std::runtime_error("Could not find a queue for graphics and present -> terminating");
	}

	// Create a chain of feature structures
	vk::StructureChain<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
		{}, // vk::PhysicalDeviceFeatures2 (empty for now)
		{.dynamicRendering = true}, // Enable dynamic rendering from Vulkan 1.3
		{.extendedDynamicState = true} // Enable extended dynamic state from the extension
	};

	float queuePriority = 0.5f;
	vk::DeviceQueueCreateInfo deviceQueueCreateInfo{
		.queueFamilyIndex = mGraphicsQueueFamilyIndex,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority
	};

	vk::DeviceCreateInfo deviceCreateInfo{
		.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>(),
		.queueCreateInfoCount = 1,
		.pQueueCreateInfos = &deviceQueueCreateInfo,
		.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size()),
		.ppEnabledExtensionNames = deviceExtensions.data()
	};

	mDevice = vk::raii::Device(mPhysicalDevice, deviceCreateInfo);
	mGraphicsQueue = vk::raii::Queue(mDevice, mGraphicsQueueFamilyIndex, 0);
}

void Renderer::createSurface() {
	VkSurfaceKHR surface;
	if (glfwCreateWindowSurface(*mInstance, mWindow->nativeHandle(), nullptr, &surface) != 0) {
		throw std::runtime_error("Failed to create window surface!");
	}

	mSurface = vk::raii::SurfaceKHR(mInstance, surface);
}

void Renderer::createSwapchain() {
	const vk::SurfaceCapabilitiesKHR surfaceCapabilities = mPhysicalDevice.getSurfaceCapabilitiesKHR(*mSurface);
	mSwapChainExtent = chooseSwapExtent(surfaceCapabilities);
	const uint32_t minImageCount = chooseSwapMinImageCount(surfaceCapabilities);

	const std::vector<vk::SurfaceFormatKHR> availableFormats = mPhysicalDevice.getSurfaceFormatsKHR(*mSurface);
	const vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(availableFormats);
	mSwapChainSurfaceFormat = surfaceFormat;

	const std::vector<vk::PresentModeKHR> availablePresentModes = mPhysicalDevice.getSurfacePresentModesKHR(*mSurface);
	const vk::PresentModeKHR presentMode = chooseSwapPresentMode(availablePresentModes);

	vk::SwapchainCreateInfoKHR swapChainCreateInfo{
		.surface = *mSurface,
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = mSwapChainExtent,
		.imageArrayLayers = 1,
		.imageUsage = vk::ImageUsageFlagBits::eColorAttachment,
		.imageSharingMode = vk::SharingMode::eExclusive,
		.preTransform = surfaceCapabilities.currentTransform,
		.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque,
		.presentMode = presentMode,
		.clipped = true
	};

	swapChainCreateInfo.oldSwapchain = nullptr;

	mSwapChain = vk::raii::SwapchainKHR(mDevice, swapChainCreateInfo);
	mSwapChainImages = mSwapChain.getImages();
}

void Renderer::createImageViews() {
	assert(mSwapChainImageViews.empty());

	vk::ImageViewCreateInfo imageViewCreateInfo{
		.viewType = vk::ImageViewType::e2D,
		.format = mSwapChainSurfaceFormat.format,
		.components = {
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity,
			vk::ComponentSwizzle::eIdentity
		},
		.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
	};

	for (const auto& image: mSwapChainImages) {
		imageViewCreateInfo.image = image;
		mSwapChainImageViews.emplace_back(mDevice, imageViewCreateInfo);
	}
}

void Renderer::createGraphicsPipeline() {
	const auto shaderPath = std::filesystem::path(SHADER_BINARY_DIR) / "triangle.spv";
	const auto shaderCode = fs::readFile(shaderPath);
	const auto shaderModule = createShaderModule(shaderCode);

	const vk::PipelineShaderStageCreateInfo vertShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eVertex,
		.module = shaderModule,
		.pName = "vertMain"
	};
	const vk::PipelineShaderStageCreateInfo fragShaderStageInfo{
		.stage = vk::ShaderStageFlagBits::eFragment,
		.module = shaderModule,
		.pName = "fragMain"
	};

	vk::PipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

	vk::PipelineVertexInputStateCreateInfo vertexInputInfo;

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{
		.topology = vk::PrimitiveTopology::eTriangleList
	};

	vk::PipelineViewportStateCreateInfo viewportState{.viewportCount = 1, .scissorCount = 1};

	vk::PipelineRasterizationStateCreateInfo rasterizer{
		.depthClampEnable = vk::False,
		.rasterizerDiscardEnable = vk::False,
		.polygonMode = vk::PolygonMode::eFill,
		.cullMode = vk::CullModeFlagBits::eBack,
		.frontFace = vk::FrontFace::eClockwise,
		.depthBiasEnable = vk::False,
		.lineWidth = 1.0f
	};

	vk::PipelineMultisampleStateCreateInfo multisampling{
		.rasterizationSamples = vk::SampleCountFlagBits::e1,
		.sampleShadingEnable = vk::False
	};

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		.blendEnable = vk::False,
		.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
		                  vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA
	};

	vk::PipelineColorBlendStateCreateInfo colorBlending{
		.logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1,
		.pAttachments = &colorBlendAttachment
	};

	std::array<vk::DynamicState, 2> dynamicStates = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};

	vk::PipelineDynamicStateCreateInfo dynamicState{
		.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		.pDynamicStates = dynamicStates.data()
	};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{.setLayoutCount = 0, .pushConstantRangeCount = 0};
	mPipelineLayout = vk::raii::PipelineLayout(mDevice, pipelineLayoutInfo);

	vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
		{
			.stageCount = 2,
			.pStages = shaderStages,
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssembly,
			.pViewportState = &viewportState,
			.pRasterizationState = &rasterizer,
			.pMultisampleState = &multisampling,
			.pColorBlendState = &colorBlending,
			.pDynamicState = &dynamicState,
			.layout = mPipelineLayout,
			.renderPass = nullptr
		},
		{.colorAttachmentCount = 1, .pColorAttachmentFormats = &mSwapChainSurfaceFormat.format}
	};

	mGraphicsPipeline = vk::raii::Pipeline(
		mDevice,
		nullptr,
		pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
}

void Renderer::createCommandPool() {
	const vk::CommandPoolCreateInfo poolInfo{
		.flags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		.queueFamilyIndex = mGraphicsQueueFamilyIndex
	};

	mCommandPool = vk::raii::CommandPool(mDevice, poolInfo);
}

void Renderer::createCommandBuffer() {
	const vk::CommandBufferAllocateInfo allocInfo{
		.commandPool = mCommandPool,
		.level = vk::CommandBufferLevel::ePrimary,
		.commandBufferCount = 1
	};

	mCommandBuffer = std::move(vk::raii::CommandBuffers(mDevice, allocInfo).front());
}

void Renderer::recordCommandBuffer(const uint32_t imageIndex) const {
	mCommandBuffer.begin({});

	// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
	transitionImageLayout(
		imageIndex,
		vk::ImageLayout::eUndefined,
		vk::ImageLayout::eColorAttachmentOptimal,
		{}, // srcAccessMask (no need to wait for previous operations)
		vk::AccessFlagBits2::eColorAttachmentWrite, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eColorAttachmentOutput // dstStage
	);

	constexpr vk::ClearValue clearColor = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);

	vk::RenderingAttachmentInfo attachmentInfo = {
		.imageView = mSwapChainImageViews[imageIndex],
		.imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		.loadOp = vk::AttachmentLoadOp::eClear,
		.storeOp = vk::AttachmentStoreOp::eStore,
		.clearValue = clearColor
	};

	const vk::RenderingInfo renderingInfo = {
		.renderArea = {.offset = {0, 0}, .extent = mSwapChainExtent},
		.layerCount = 1,
		.colorAttachmentCount = 1,
		.pColorAttachments = &attachmentInfo
	};

	mCommandBuffer.beginRendering(renderingInfo);
	mCommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *mGraphicsPipeline);
	mCommandBuffer.setViewport(
		0,
		vk::Viewport(0.0f, 0.0f,
		             static_cast<float>(mSwapChainExtent.width),
		             static_cast<float>(mSwapChainExtent.height),
		             0.0f, 1.0f)
	);
	mCommandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), mSwapChainExtent));
	mCommandBuffer.draw(3, 1, 0, 0);
	mCommandBuffer.endRendering();

	// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
	transitionImageLayout(
		imageIndex,
		vk::ImageLayout::eColorAttachmentOptimal,
		vk::ImageLayout::ePresentSrcKHR,
		vk::AccessFlagBits2::eColorAttachmentWrite, // srcAccessMask
		{}, // dstAccessMask
		vk::PipelineStageFlagBits2::eColorAttachmentOutput, // srcStage
		vk::PipelineStageFlagBits2::eBottomOfPipe // dstStage
	);
	mCommandBuffer.end();
}

void Renderer::transitionImageLayout(
	const uint32_t imageIndex,
	const vk::ImageLayout oldLayout,
	const vk::ImageLayout newLayout,
	const vk::AccessFlags2 srcAccessMask,
	const vk::AccessFlags2 dstAccessMask,
	const vk::PipelineStageFlags2 srcStageMask,
	const vk::PipelineStageFlags2 dstStageMask) const {
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = mSwapChainImages[imageIndex],
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	const vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	mCommandBuffer.pipelineBarrier2(dependency_info);
}

void Renderer::createSyncObjects() {
	mPresentCompleteSemaphore = vk::raii::Semaphore(mDevice, vk::SemaphoreCreateInfo());
	mRenderFinishedSemaphore = vk::raii::Semaphore(mDevice, vk::SemaphoreCreateInfo());
	mDrawFence = vk::raii::Fence(mDevice, {.flags = vk::FenceCreateFlagBits::eSignaled});
}

void Renderer::getPhysicalDevice() {
	const auto physicalDevices = mInstance.enumeratePhysicalDevices();

	if (physicalDevices.empty()) {
		throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	}

	for (auto& phyDevice: physicalDevices) {
		if (checkDeviceSuitable(phyDevice)) {
			mPhysicalDevice = phyDevice;
			break;
		}
	}
}

std::vector<const char*> Renderer::getRequiredInstanceExtensions() {
	uint32_t glfwExtensionCount = 0;
	const auto glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (enableValidationLayers) {
		extensions.push_back(vk::EXTDebugUtilsExtensionName);
	}

	return extensions;
}

vk::Bool32 Renderer::debugCallback(
	vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
	const vk::DebugUtilsMessageTypeFlagsEXT type,
	const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	std::cerr << "validation layer: type " << to_string(type) << " msg: " << pCallbackData->pMessage << std::endl;

	return vk::False;
}

bool Renderer::checkDeviceSuitable(const vk::raii::PhysicalDevice& phyDevice) {
	// Check if the physicalDevice supports the Vulkan 1.3 API version
	bool supportsVulkan1_3 = phyDevice.getProperties().apiVersion >= vk::ApiVersion13;

	// Check if any of the queue families support graphics operations
	bool supportsGraphics{false};
	for (const auto& qfp: phyDevice.getQueueFamilyProperties()) {
		if (qfp.queueFlags & vk::QueueFlagBits::eGraphics) {
			supportsGraphics = true;
			break;
		}
	}

	// Check if all required physicalDevice extensions are available
	std::unordered_set<std::string_view> availableSet;
	for (const auto& [extensionName, specVersion]: phyDevice.enumerateDeviceExtensionProperties()) {
		availableSet.insert(extensionName);
	}

	bool supportsAllRequiredExtensions{true};
	for (const char* required: deviceExtensions) {
		if (!availableSet.contains(required)) {
			supportsAllRequiredExtensions = false;
			break;
		}
	}

	// Check if the physicalDevice supports the required features (dynamic rendering and extended dynamic state)
	auto features2 = phyDevice.getFeatures2<
		vk::PhysicalDeviceFeatures2,
		vk::PhysicalDeviceVulkan13Features,
		vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	// 2. Extract the specific feature blocks
	const auto& vk13Features = features2.get<vk::PhysicalDeviceVulkan13Features>();
	const auto& extDynamicFeatures = features2.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

	// 3. Perform the checks with clear boolean logic
	bool supportsDynamicRendering = vk13Features.dynamicRendering;
	bool supportsExtendedDynamicState = extDynamicFeatures.extendedDynamicState;
	bool supportsRequiredFeatures = supportsDynamicRendering && supportsExtendedDynamicState;

	return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
}

vk::SurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
	for (const auto& format: availableFormats) {
		if (format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return format;
		}
	}

	return availableFormats[0];
}

vk::PresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
	for (const auto& mode: availablePresentModes) {
		if (mode == vk::PresentModeKHR::eMailbox) {
			return mode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D Renderer::chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(mWindow->nativeHandle(), &width, &height);

	return {
		std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
	};
}

uint32_t Renderer::chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities) {
	auto minImageCount = std::max(3u, surfaceCapabilities.minImageCount);

	if ((0 < surfaceCapabilities.maxImageCount) && (surfaceCapabilities.maxImageCount < minImageCount)) {
		minImageCount = surfaceCapabilities.maxImageCount;
	}

	return minImageCount;
}

vk::raii::ShaderModule Renderer::createShaderModule(const std::vector<char>& code) const {
	const vk::ShaderModuleCreateInfo createInfo{
		.codeSize = code.size() * sizeof(char),
		.pCode = reinterpret_cast<const uint32_t*>(code.data())
	};

	vk::raii::ShaderModule shaderModule{mDevice, createInfo};
	return shaderModule;
}
