#include "VulkanRenderer.h"

VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {
    
}

int VulkanRenderer::Init(SDL_Window *window) {
    m_window = window;
    
    try {
        CreateInstance();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
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
    
    uint32_t requiredExtensionCount = 0;
    std::vector<const char*> requiredExtensions;
    
    
    SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, nullptr);
    requiredExtensions.resize(requiredExtensionCount);
    
    if (SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, requiredExtensions.data()) == SDL_FALSE) {
        throw std::runtime_error("Failed to get Instance Extensions");
    }
    
    requiredExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
    
    VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
        &appInfo,
        0,
        nullptr,
        static_cast<uint32_t>(requiredExtensions.size()),
        requiredExtensions.data()
    };
    
    VkResult result = vkCreateInstance(&createInfo, nullptr, &m_instance);
    
    if(result != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
        
    }
}



