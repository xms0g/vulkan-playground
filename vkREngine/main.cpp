#include "vulkanRenderer.h"


int main(int argc, const char * argv[]) {
    VulkanRenderer renderer{};
    Window window{};
    
    try {
        window.init("vkREngine");
        renderer.init(&window);
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
   
    while(!glfwWindowShouldClose(window.nativeHandle())) {
           glfwPollEvents();
    }
    
    return 0;
}
