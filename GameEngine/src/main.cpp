#include <iostream>
#include "EngineApplication.h"

int RunApp() {
    try {
        JoeEngine::JEEngineApplication app = JoeEngine::JEEngineApplication();
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
