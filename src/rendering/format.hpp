#pragma once
#include <vector>
#include <vulkan/vulkan_raii.hpp>

namespace Format {
inline vk::Format findSupportedFormat(
const vk::raii::PhysicalDevice& phyDev,
	const std::vector<vk::Format>& candidates,
	const vk::ImageTiling tiling,
	const vk::FormatFeatureFlags features) {
	for (const auto& format: candidates) {
		const vk::FormatProperties props = phyDev.getFormatProperties(format);

		if ((tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) ||
			(tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features)) {
			return format;
			}
	}

	throw std::runtime_error("Failed to find supported format!");
}

inline vk::Format findDepthFormat(const vk::raii::PhysicalDevice& phyDev) {
	return findSupportedFormat(
	phyDev,
		{vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint},
		vk::ImageTiling::eOptimal,
		vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
}
