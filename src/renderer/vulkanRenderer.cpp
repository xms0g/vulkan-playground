#include "vulkanRenderer.h"
#include <cstring>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <unordered_set>
#include <GLFW/glfw3.h>
#include "deviceExtension.hpp"
#include "vulkanValidation.hpp"
#include "../core/window.h"
#include "../../libs/filesystem/filesystem.h"

VulkanRenderer::~VulkanRenderer() {
}

int VulkanRenderer::init(Window* window) {
	mWindow = window;

	try {
		createInstance();
		createSurface();
		getPhysicalDevice();
		createLogicalDevice();
		createSwapchain();
		createRenderPass();
		createGraphicsPipeline();
	} catch (const std::runtime_error& e) {
		throw std::runtime_error(e.what());
	}
	return 0;
}

void VulkanRenderer::createInstance() {
	constexpr vk::ApplicationInfo appInfo{
		.pApplicationName = "Hello Triangle",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName = "No Engine",
		.engineVersion = VK_MAKE_VERSION(1, 0, 0),
		.apiVersion = vk::ApiVersion14
	};

	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	if (glfwExtensions == nullptr) {
		throw std::runtime_error("Failed to get Instance Extensions!");
	}

	std::unordered_set<std::string> supported;
	for (const auto& [extensionName, specVersion] : context.enumerateInstanceExtensionProperties()) {
		supported.insert(extensionName);
	}

	std::vector<const char*> requiredExtensions;
	for (uint32_t i = 0; i < glfwExtensionCount; ++i) {
		if (!supported.contains(glfwExtensions[i])) {
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(glfwExtensions[i]));
		}
		requiredExtensions.emplace_back(glfwExtensions[i]);
	}

	vk::InstanceCreateFlagBits flags{0};
#ifdef __APPLE__
	requiredExtensions.emplace_back(vk::KHRPortabilityEnumerationExtensionName);
	flags = vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR;
#endif

	const vk::InstanceCreateInfo createInfo{
		.flags = flags,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size()),
		.ppEnabledExtensionNames = requiredExtensions.data()
	};

	instance = vk::raii::Instance(context, createInfo);
}

void VulkanRenderer::createLogicalDevice() {
}

void VulkanRenderer::createSurface() {
}

void VulkanRenderer::createSwapchain() {
}

void VulkanRenderer::createRenderPass() {
}

void VulkanRenderer::createGraphicsPipeline() {
}

void VulkanRenderer::getPhysicalDevice() {
}

QueueFamilyIndices VulkanRenderer::getQueueFamilies(VkPhysicalDevice device) {
}

SwapchainDetails VulkanRenderer::getSwapChainDetails(VkPhysicalDevice device) {
}

bool VulkanRenderer::checkInstanceExtensionSupport(const std::vector<const char*>& checkExtensions) {
}


bool VulkanRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
}

bool VulkanRenderer::checkValidationLayerSupport(const std::vector<const char*>& checkLayers) {
}


bool VulkanRenderer::checkDeviceSuitable(VkPhysicalDevice device) {
}

VkSurfaceFormatKHR VulkanRenderer::chooseBestSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
}

VkPresentModeKHR VulkanRenderer::chooseBestPresentationMode(const std::vector<VkPresentModeKHR>& modes) {
}

VkExtent2D VulkanRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& surfaceCapabilities) {
}

VkImageView VulkanRenderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
}

VkShaderModule VulkanRenderer::createShaderModule(const std::vector<char>& code) {
}
