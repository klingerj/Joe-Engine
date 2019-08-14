#pragma once

#include <string>

#include "io/IOHandler.h"
#include "scene/SceneManager.h"
#include "rendering/VulkanRenderer.h"
#include "physics/PhysicsManager.h"

namespace JoeEngine {
    class JEEngineApplication {
    private:
        // Subsystems
        JESceneManager m_sceneManager;
        JEVulkanRenderer m_vulkanRenderer;
        JEIOHandler m_ioHandler;
        JEPhysicsManager m_physicsManager;

        double m_frameStartTime, m_frameEndTime; // timing for performance analysis
        bool m_enableFrameCounter;

        // Initialize subsystems
        void InitializeEngine();

    public:
        JEEngineApplication() : m_enableFrameCounter(false), m_frameStartTime(0.0f), m_frameEndTime(0.0f) {
            InitializeEngine();
        }

        // Destroy resources/subsystems
        ~JEEngineApplication() {}

        // Run the main loop
        void Run();

        // Shutdown engine
        void StopEngine();

        // Getters
        JEVulkanRenderer* GetRenderSubsystem() {
            return &m_vulkanRenderer;
        }
        JEIOHandler* GetIOSubsystem() {
            return &m_ioHandler;
        }
    };
}
