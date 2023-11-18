#include "vulkanRenderer.h"


VulkanRenderer::VulkanRenderer() {

}

VulkanRenderer::~VulkanRenderer() {
    vkDestroyInstance(m_instance, nullptr);
}

int VulkanRenderer::init(SDL_Window *window) {
    m_window = window;
    
    try {
        createInstance();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return 0;
}

void VulkanRenderer::createInstance() {
    VkApplicationInfo appInfo = {
        VK_STRUCTURE_TYPE_APPLICATION_INFO,
        nullptr,
        "vkREngine",                // Custom name of the application
        VK_MAKE_VERSION(1, 0, 0),   // Custom version of the application
        "No Engine",                // Custom engine name
        VK_MAKE_VERSION(1, 0, 0),   // Custom engine version
        VK_API_VERSION_1_3          // The vulkan version
    };
    
    uint32_t requiredExtensionCount = 0;
    std::vector<const char*> requiredExtensions;
        
    SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, nullptr);
    requiredExtensions.resize(requiredExtensionCount);
    
    SDL_bool result = SDL_Vulkan_GetInstanceExtensions(m_window, &requiredExtensionCount, requiredExtensions.data());
    if (result == SDL_FALSE) {
        throw std::runtime_error("Failed to get Instance Extensions");
    }
#ifdef __APPLE__
    requiredExtensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#endif
    
    VkInstanceCreateInfo createInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,             //sType
        nullptr,                                            //pNext
#ifdef __APPLE__
        VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,   //flags
#else
        0
#endif
        &appInfo,                                           //pApplicationInfo
        0,                                                  //enabledLayerCount
        nullptr,                                            //ppEnabledLayerNames
        static_cast<uint32_t>(requiredExtensions.size()),   //enabledExtensionCount
        requiredExtensions.data()                           //ppEnabledExtensionNames
    };
    
    if(vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan Instance");
        
    }
}



