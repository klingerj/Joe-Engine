#include <iostream>
#include "EngineInstance.h"
#include "Components/Rotator/RotatorComponentManager.h"

int RunApp() {
    try {
        JoeEngine::JEEngineInstance app = JoeEngine::JEEngineInstance(JoeEngine::RendererSettings::AllSettings);
        app.RegisterComponentManager<RotatorComponent>(new RotatorComponentManager());
        app.LoadScene(0);
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
