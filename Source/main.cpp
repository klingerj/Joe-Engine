#include <iostream>
#include "EngineInstance.h"

int RunApp() {
    try {
        JoeEngine::JEEngineInstance app = JoeEngine::JEEngineInstance();
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int main() {
    return RunApp();
}
