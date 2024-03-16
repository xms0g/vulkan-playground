#include "engine.h"


int main(int argc, const char* argv[]) {
    try {
        Engine vkengine;
        vkengine.run();
    } catch (const std::runtime_error& e) {
        std::cerr << "ERROR: " << e.what() << "\n";
    }
    return 0;
}
