#include <stdio.h>
#include <iostream>
#include "EngineApplication.h"

int main() {
    // Run the app
    try {
        EngineApplication app = EngineApplication();
        app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
