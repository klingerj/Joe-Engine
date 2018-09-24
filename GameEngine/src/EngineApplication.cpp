#include "EngineApplication.h"
#include <exception>
void EngineApplication::Run() {
    const VulkanWindow& window = vulkanDevice.GetWindow();
    while (!window.ShouldClose()) {
        window.PollEvents();
        break;
    }
}

void EngineApplication::InitializeEngine() {
    // Initialize subsystems?
}
