#include "vulkanRenderer.h"


int main(int argc, const char * argv[]) {
    VulkanRenderer renderer{};
    Window window{};
    double deltaTime{};
    double millisecsPreviousFrame{0};
    
    try {
        window.init("vkREngine");
        renderer.init(&window);
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
   
    while(!window.shouldClose()) {
        glfwPollEvents();

        deltaTime = (glfwGetTime() - millisecsPreviousFrame) / 1000.0f;
        millisecsPreviousFrame = glfwGetTime();
        //window.updateFpsCounter(deltaTime);
        
        window.swapBuffer();
    }
    
    return 0;
}
