#include <iostream>
#include "EngineInstance.h"
#include "Components/Rotator/RotatorComponentManager.h"

int RunApp() {
    try {
        JoeEngine::RendererSettings rendererSettings = JoeEngine::RendererSettings::EnableDeferred;
        rendererSettings = rendererSettings | JoeEngine::RendererSettings::EnableOIT;
        JoeEngine::JEEngineInstance app = JoeEngine::JEEngineInstance(rendererSettings);
        app.RegisterComponentManager<RotatorComponent, RotatorComponentManager>();
        app.LoadScene(2);
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
