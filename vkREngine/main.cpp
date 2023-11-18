#include "vulkanRenderer.h"


SDL_Window* sdlWindow;
VulkanRenderer renderer;

void initWindow(const char* title = "Test Window", int width = 800, int height = 600) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error initializing SDL" << std::endl;
        return;
    }
    
    sdlWindow = SDL_CreateWindow(title,
                                 SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED, height, width,
                                 SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);
    
}

int main(int argc, const char * argv[]) {
    
    initWindow();
    
    if (renderer.init(sdlWindow) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    
    SDL_Event event;
    bool running = true;
    while(running) {
        while(SDL_PollEvent(&event)) {
            if(event.type == SDL_QUIT) {
                running = false;
            }
        }
    }
    
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
    return 0;
}
