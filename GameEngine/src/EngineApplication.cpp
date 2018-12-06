#include <exception>

#include "EngineApplication.h"

void EngineApplication::Run() {
    const VulkanWindow& window = vulkanRenderer.GetWindow();
    ioHandler.Initialize(window.GetWindow());

    while (!window.ShouldClose()) {
        frameStartTime = glfwGetTime();
        ioHandler.PollInput();
        vulkanRenderer.DrawFrame();
        //physicsManager.Update();
        frameEndTime = glfwGetTime();
        if (enableFrameCounter) {
            std::cout << "Frame Time: " << (frameEndTime - frameStartTime) * 1000.0f << " ms" << std::endl;
        }
    }

    vkDeviceWaitIdle(vulkanRenderer.GetDevice());
    StopEngine();
}

void EngineApplication::InitializeEngine() {
    std::shared_ptr<MeshDataManager> meshDataManager = std::make_shared<MeshDataManager>();
    physicsManager.Initialize(meshDataManager);
    sceneManager.Initialize(meshDataManager);
    vulkanRenderer.Initialize(&sceneManager);

    GLFWwindow* window = vulkanRenderer.GetGLFWWindow();
    ioHandler.Initialize(window);
    glfwSetWindowUserPointer(window, this);
    sceneManager.RegisterCallbacks(&ioHandler);
}

void EngineApplication::StopEngine() {
    vulkanRenderer.Cleanup();
}
