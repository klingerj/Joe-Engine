#include <exception>

#include "EngineApplication.h"

void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetWindow();

    while (!window.ShouldClose()) {
        frameStartTime = glfwGetTime();
        window.PollEvents();
        vulkanRenderer.DrawFrame();
        frameEndTime = glfwGetTime();
        if (enableFrameCounter) {
            std::cout << "Frame Time: " << (frameEndTime - frameStartTime) * 1000.0f << " ms" << std::endl;
        }
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
