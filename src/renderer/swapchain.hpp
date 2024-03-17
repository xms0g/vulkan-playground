#pragma once

struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainImage {
    VkImage image;
    VkImageView imageView;
};
