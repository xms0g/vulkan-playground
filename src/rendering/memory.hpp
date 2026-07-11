#pragma once
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>

namespace Memory {
inline uint32_t findMemoryType(
	const uint32_t typeFilter,
	vk::MemoryPropertyFlags properties,
	const vk::raii::PhysicalDevice& phyDev) {
	const vk::PhysicalDeviceMemoryProperties memProperties = phyDev.getMemoryProperties();

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
}
