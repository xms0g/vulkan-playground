#pragma once

#include <fstream>

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
// Indices(locations) of Queue Families
struct QueueFamilyIndices {
    int graphicsFamily = -1;        // Location of the Graphics Queue Family
    int presentationFamily = -1;    // Location of the Presentation Queue Family

    [[nodiscard]] bool isValid() const {
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


static std::vector<char> readFile(const std::string& fileName) {
    std::ifstream file{fileName, std::ios::binary | std::ios::ate};

    if (!file.is_open()){
        throw std::runtime_error("Failed to read file: " + fileName);
    }

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> fileBuffer(fileSize);

    file.seekg(0);

    file.read(fileBuffer.data(), fileSize);

    file.close();

    return fileBuffer;
}
