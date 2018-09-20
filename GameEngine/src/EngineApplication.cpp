#include "EngineApplication.h"
#include <exception>
void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetDevice().GetWindow();
    while (!window.ShouldClose()) {
        window.PollEvents();
        break;
    }
}

void EngineApplication::InitializeEngine() {
    // Initialize stuff
}
