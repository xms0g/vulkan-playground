#pragma once

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
// Indices(locations) of Queue Families
struct QueueFamilyIndices {
    int graphicsFamily = -1;        // Location of the Graphics Queue Family
    int presentationFamily = -1;    // Location of the Presentation Queue Family

    bool isValid() {
        return graphicsFamily >= 0 && presentationFamily >= 0;
    }
};

struct SwapchainDetails {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

struct SwapchainImage {
    VkImage image;
    VkImageView imageView;
};
