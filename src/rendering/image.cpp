#include "image.h"
#include "commandBuffer.h"
#include "memory.h"

Image::Image(
	const vk::raii::Device& device,
	const vk::raii::PhysicalDevice& phyDev,
	const uint32_t width,
	const uint32_t height,
	const uint32_t mipLevels,
	const vk::SampleCountFlagBits numSamples,
	const vk::Format format,
	const vk::ImageTiling tiling,
	const vk::ImageUsageFlags usage,
	const vk::MemoryPropertyFlags properties)
	: mFormat(format),
	  mWidth(width),
	  mHeight(height),
	  mMipLevels(mipLevels) {
	const vk::ImageCreateInfo imageInfo{
		.imageType = vk::ImageType::e2D,
		.format = format,
		.extent = {width, height, 1},
		.mipLevels = mipLevels,
		.arrayLayers = 1,
		.samples = numSamples,
		.tiling = tiling,
		.usage = usage, .
		sharingMode = vk::SharingMode::eExclusive
	};

	mImage = vk::raii::Image(device, imageInfo);

	const vk::MemoryRequirements memRequirements = mImage.getMemoryRequirements();
	mImageMemory = DeviceMemory(device, phyDev, memRequirements.size, memRequirements.memoryTypeBits, properties);

	mImage.bindMemory(*mImageMemory, 0);
}

void Image::createImageView(const vk::raii::Device& device, const vk::ImageAspectFlags aspectFlags) {
	const vk::ImageViewCreateInfo viewInfo{
		.image = mImage,
		.viewType = vk::ImageViewType::e2D,
		.format = mFormat,
		.subresourceRange = {aspectFlags, 0, mMipLevels, 0, 1}
	};

	mImageView = vk::raii::ImageView(device, viewInfo);
}

void Image::generateMipmaps(const vk::raii::PhysicalDevice& phyDev, const vk::raii::CommandBuffer& cmd) const {
	const vk::FormatProperties formatProperties = phyDev.getFormatProperties(mFormat);

	if (!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear)) {
		throw std::runtime_error("Texture image format does not support linear blitting!");
	}

	vk::ImageMemoryBarrier2 barrier = {
		.srcAccessMask = vk::AccessFlagBits2::eTransferWrite,
		.dstAccessMask = vk::AccessFlagBits2::eTransferRead,
		.oldLayout = vk::ImageLayout::eTransferDstOptimal,
		.newLayout = vk::ImageLayout::eTransferSrcOptimal,
		.srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		.dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		.image = mImage,
		.subresourceRange = {
			.aspectMask = vk::ImageAspectFlagBits::eColor,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		}
	};

	const vk::DependencyInfo dependencyInfo = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	int32_t mipWidth = mWidth;
	int32_t mipHeight = mHeight;

	for (uint32_t i = 1; i < mMipLevels; ++i) {
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
		barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
		barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
		barrier.dstStageMask = vk::PipelineStageFlagBits2::eTransfer;
		barrier.dstAccessMask = vk::AccessFlagBits2::eTransferRead;

		cmd.pipelineBarrier2(dependencyInfo);

		vk::ArrayWrapper1D<vk::Offset3D, 2> srcOffsets, dstOffsets;
		srcOffsets[0] = {0, 0, 0};
		srcOffsets[1] = {mipWidth, mipHeight, 1};
		dstOffsets[0] = {0, 0, 0};
		dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};

		const vk::ImageBlit blit = {
			.srcSubresource = {vk::ImageAspectFlagBits::eColor, i - 1, 0, 1},
			.srcOffsets = srcOffsets,
			.dstSubresource = {vk::ImageAspectFlagBits::eColor, i, 0, 1},
			.dstOffsets = dstOffsets
		};

		cmd.blitImage(
			mImage,
			vk::ImageLayout::eTransferSrcOptimal,
			mImage,
			vk::ImageLayout::eTransferDstOptimal,
			{blit},
			vk::Filter::eLinear);

		barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
		barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
		barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
		barrier.srcAccessMask = vk::AccessFlagBits2::eTransferRead;
		barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
		barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

		cmd.pipelineBarrier2(dependencyInfo);

		if (mipWidth > 1) mipWidth /= 2;
		if (mipHeight > 1) mipHeight /= 2;
	}

	barrier.subresourceRange.baseMipLevel = mMipLevels - 1;
	barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
	barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
	barrier.srcStageMask = vk::PipelineStageFlagBits2::eTransfer;
	barrier.srcAccessMask = vk::AccessFlagBits2::eTransferWrite;
	barrier.dstStageMask = vk::PipelineStageFlagBits2::eFragmentShader;
	barrier.dstAccessMask = vk::AccessFlagBits2::eShaderRead;

	cmd.pipelineBarrier2(dependencyInfo);
}

void Image::transitionImageLayout(
	const vk::Image image,
	const vk::ImageLayout oldLayout,
	const vk::ImageLayout newLayout,
	const vk::AccessFlags2 srcAccessMask,
	const vk::AccessFlags2 dstAccessMask,
	const vk::PipelineStageFlags2 srcStageMask,
	const vk::PipelineStageFlags2 dstStageMask,
	const vk::ImageAspectFlags aspectFlags,
	const vk::raii::CommandBuffer& cmd,
	const uint32_t mipLevels) {
	vk::ImageMemoryBarrier2 barrier = {
		.srcStageMask = srcStageMask,
		.srcAccessMask = srcAccessMask,
		.dstStageMask = dstStageMask,
		.dstAccessMask = dstAccessMask,
		.oldLayout = oldLayout,
		.newLayout = newLayout,
		.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		.image = image,
		.subresourceRange = {
			.aspectMask = aspectFlags,
			.baseMipLevel = 0,
			.levelCount = mipLevels,
			.baseArrayLayer = 0,
			.layerCount = 1
		}
	};

	const vk::DependencyInfo dependency_info = {
		.dependencyFlags = {},
		.imageMemoryBarrierCount = 1,
		.pImageMemoryBarriers = &barrier
	};

	cmd.pipelineBarrier2(dependency_info);
}
