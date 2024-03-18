#pragma once

#include <vector>
#include <vulkan/vulkan.h>

struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainImage {
    VkImage image;
    VkImageView imageView;
};
