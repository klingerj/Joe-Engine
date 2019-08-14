#include <exception>

#include "EngineApplication.h"

namespace JoeEngine {
    void JEEngineApplication::Run() {
        const JEVulkanWindow& window = m_vulkanRenderer.GetWindow();
        m_ioHandler.Initialize(window.GetWindow());

        while (!window.ShouldClose()) {
            m_frameStartTime = glfwGetTime();
            m_ioHandler.PollInput();
            m_vulkanRenderer.DrawFrame();
            m_physicsManager.Update();
            m_frameEndTime = glfwGetTime();
            if (m_enableFrameCounter) {
                std::cout << "Frame Time: " << (m_frameEndTime - m_frameStartTime) * 1000.0f << " ms" << std::endl;
            }
        }

        vkDeviceWaitIdle(m_vulkanRenderer.GetDevice());
        StopEngine();
    }

    void JEEngineApplication::InitializeEngine() {
        std::shared_ptr<JEMeshDataManager> meshDataManager = std::make_shared<JEMeshDataManager>();
        m_physicsManager.Initialize(meshDataManager);
        m_sceneManager.Initialize(meshDataManager);
        m_vulkanRenderer.Initialize(&m_sceneManager);

        GLFWwindow* window = m_vulkanRenderer.GetGLFWWindow();
        m_ioHandler.Initialize(window);
        glfwSetWindowUserPointer(window, this);
        m_vulkanRenderer.RegisterCallbacks(&m_ioHandler);
        m_sceneManager.RegisterCallbacks(&m_ioHandler);
    }

    void JEEngineApplication::StopEngine() {
        m_vulkanRenderer.Cleanup();
    }
}
