#include "EngineApplication.h"
#include <exception>
void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetWindow();
    while (!window.ShouldClose()) {
        window.PollEvents();
        break;
    }
    StopEngine();
}

void EngineApplication::InitializeEngine() {
    vulkanRenderer.Initialize();
}

void EngineApplication::StopEngine() {
    vulkanRenderer.Cleanup();
}
