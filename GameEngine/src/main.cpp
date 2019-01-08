#include <iostream>
#include "EngineApplication.h"

int RunApp() {
    try {
        JEEngineApplication app = JEEngineApplication();
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
