#pragma once
#include <GLFW/glfw3.h>
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

class Swapchain {
public:
	Swapchain() = default;

	Swapchain(const vk::raii::SurfaceKHR& surface,
	          const vk::raii::Device& device,
	          const vk::raii::PhysicalDevice& phyDev,
	          GLFWwindow& window);

	[[nodiscard]]
	const vk::SurfaceFormatKHR& surfaceFormat() const;

	[[nodiscard]]
	const vk::Image& image(uint32_t imageIndex) const;

	[[nodiscard]]
	size_t imageCount() const;

	[[nodiscard]]
	const vk::raii::ImageView& imageView(uint32_t imageIndex) const;

	[[nodiscard]]
	const vk::Extent2D& extent() const;

	[[nodiscard]]
	uint32_t acquireNextImage(const vk::raii::Semaphore& sem) const;

	void recreate(const vk::raii::SurfaceKHR& surface,
	              const vk::raii::Device& device,
	              const vk::raii::PhysicalDevice& phyDev,
	              GLFWwindow& window);

	vk::raii::SwapchainKHR& operator*() noexcept { return mSwapChain; }
	const vk::raii::SwapchainKHR& operator*() const noexcept { return mSwapChain; }

private:
	void create(const vk::raii::SurfaceKHR& surface,
	            const vk::raii::Device& device,
	            const vk::raii::PhysicalDevice& phyDev,
	            GLFWwindow& window);

	void createSwapchainImageViews(const vk::raii::Device& device);

	static vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats);

	static vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes);

	static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities, GLFWwindow& window);

	static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR& surfaceCapabilities);

	vk::SurfaceFormatKHR mSwapChainSurfaceFormat;
	vk::Extent2D mSwapChainExtent;
	std::vector<vk::Image> mSwapChainImages;
	std::vector<vk::raii::ImageView> mSwapChainImageViews;
	vk::raii::SwapchainKHR mSwapChain{nullptr};
};
