#include "vulkanRenderer.h"


GLFWwindow* window;
VulkanRenderer renderer;

void initWindow(const char* title = "vkREngine", int width = 800, int height = 600) {
    if (glfwInit() != GLFW_TRUE) {
        std::cerr << "Error initializing SDL" << std::endl;
        return;
    }
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
}

int main(int argc, const char * argv[]) {
    
    initWindow();
    
    if (renderer.init(window) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    
    while(!glfwWindowShouldClose(window)) {
           glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
