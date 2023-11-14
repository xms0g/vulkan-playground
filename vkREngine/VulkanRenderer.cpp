#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {
    
}

int VulkanRenderer::Init(SDL_Window *window) {
    mWindow = window;
    
    try {
        CreateInstance();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR:  " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return 0;
}

void VulkanRenderer::CreateInstance() {
    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "Vulkan App",               // Custom name of the application
        VK_MAKE_VERSION(1, 0, 0),   // Custom version of the application
        "No Engine",                // Custom engine name
        VK_MAKE_VERSION(1, 0, 0),   // Custom engine version
        VK_VERSION_1_3              // The vulkan version
    };
    
    uint32_t sdlExtensionCount = 0;
    std::vector<const char*> sdlExtensions;
    
    SDL_Vulkan_GetInstanceExtensions(mWindow, &sdlExtensionCount, NULL);
    sdlExtensions.reserve(sdlExtensionCount);
    
    if (SDL_Vulkan_GetInstanceExtensions(mWindow, &sdlExtensionCount, sdlExtensions.data()) == SDL_FALSE) {
        throw std::runtime_error("ERROR: Cannot get instance extensions\n");
    }
    
    VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0x0,
        &appInfo,
        0,
        nullptr,
        sdlExtensionCount,
        sdlExtensions.data()
    };
    
    vkCreateInstance(&createInfo, nullptr, &instance);
    
}



