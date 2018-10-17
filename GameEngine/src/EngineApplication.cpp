#include "EngineApplication.h"
#include <exception>
void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetWindow();
    while (!window.ShouldClose()) {
        window.PollEvents();
        vulkanRenderer.DrawFrame();
    }

    vkDeviceWaitIdle(vulkanRenderer.GetDevice());
    StopEngine();
}

void EngineApplication::InitializeEngine() {
    vulkanRenderer.Initialize(&sceneManager);
}

void EngineApplication::StopEngine() {
    vulkanRenderer.Cleanup();
}
