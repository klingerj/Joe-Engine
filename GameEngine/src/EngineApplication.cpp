#include <exception>

#include "EngineApplication.h"

void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetWindow();
    ioHandler.Initialize(window.GetWindow());

    while (!window.ShouldClose()) {
        frameStartTime = glfwGetTime();
        ioHandler.PollInput();
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
    sceneManager.Initialize();
    vulkanRenderer.Initialize(&sceneManager);
    GLFWwindow* window = vulkanRenderer.GetGLFWWindow();
    ioHandler.Initialize(window);
    glfwSetWindowUserPointer(window, this);
    sceneManager.RegisterCallbacks(&ioHandler);
}

void EngineApplication::StopEngine() {
    vulkanRenderer.Cleanup();
}
