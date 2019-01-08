#include <exception>

#include "EngineApplication.h"

void JEEngineApplication::Run() {
    const JEVulkanWindow& window = vulkanRenderer.GetWindow();
    ioHandler.Initialize(window.GetWindow());

    while (!window.ShouldClose()) {
        frameStartTime = glfwGetTime();
        ioHandler.PollInput();
        vulkanRenderer.DrawFrame();
        physicsManager.Update();
        frameEndTime = glfwGetTime();
        if (enableFrameCounter) {
            std::cout << "Frame Time: " << (frameEndTime - frameStartTime) * 1000.0f << " ms" << std::endl;
        }
    }

    vkDeviceWaitIdle(vulkanRenderer.GetDevice());
    StopEngine();
}

void JEEngineApplication::InitializeEngine() {
    std::shared_ptr<JEMeshDataManager> meshDataManager = std::make_shared<JEMeshDataManager>();
    physicsManager.Initialize(meshDataManager);
    sceneManager.Initialize(meshDataManager);
    vulkanRenderer.Initialize(&sceneManager);

    GLFWwindow* window = vulkanRenderer.GetGLFWWindow();
    ioHandler.Initialize(window);
    glfwSetWindowUserPointer(window, this);
    vulkanRenderer.RegisterCallbacks(&ioHandler);
    sceneManager.RegisterCallbacks(&ioHandler);
}

void JEEngineApplication::StopEngine() {
    vulkanRenderer.Cleanup();
}
