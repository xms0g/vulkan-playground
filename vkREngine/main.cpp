#include "vulkanRenderer.h"
#include "window.h"



int main(int argc, const char * argv[]) {
    VulkanRenderer renderer;

    Window window{"vkREngine"};
    
    if (renderer.init(window.nativeHandle()) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    
    while(!glfwWindowShouldClose(window.nativeHandle())) {
           glfwPollEvents();
    }
    
    return 0;
}
