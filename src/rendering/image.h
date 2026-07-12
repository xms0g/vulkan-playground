#pragma once
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>
#include "memory.h"

class CommandBuffer;

class Image {
public:
	Image(
		const vk::raii::Device& device,
		const vk::raii::PhysicalDevice& phyDev,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		vk::SampleCountFlagBits numSamples,
		vk::Format format,
		vk::ImageTiling tiling,
		vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties);

	vk::raii::ImageView& imageView() noexcept { return mImageView; }
	const vk::raii::ImageView& imageView() const noexcept { return mImageView; }

	void createImageView(const vk::raii::Device& device, vk::ImageAspectFlags aspectFlags);

	void generateMipmaps(const vk::raii::PhysicalDevice& phyDev, const vk::raii::CommandBuffer& commandBuffer) const;

	vk::raii::Image& operator*() noexcept { return mImage; }
	const vk::raii::Image& operator*() const noexcept { return mImage; }

	static void transitionImageLayout(
		vk::Image image,
		vk::ImageLayout oldLayout,
		vk::ImageLayout newLayout,
		vk::AccessFlags2 srcAccessMask,
		vk::AccessFlags2 dstAccessMask,
		vk::PipelineStageFlags2 srcStageMask,
		vk::PipelineStageFlags2 dstStageMask,
		vk::ImageAspectFlags aspectFlags,
		const vk::raii::CommandBuffer& commandBuffer,
		uint32_t mipLevels);

private:
	vk::Format mFormat;
	uint32_t mWidth, mHeight, mMipLevels;
	vk::raii::Image mImage{nullptr};
	DeviceMemory mImageMemory;
	vk::raii::ImageView mImageView{nullptr};
};
