#pragma once

#include <string>

#include "Io/IOHandler.h"
#include "Scene/SceneManager.h"
#include "Rendering/VulkanRenderer.h"
#include "Physics/PhysicsManager.h"
#include "Scene/EntityManager.h"
#include "Components/Mesh/MeshComponentManager.h"
#include "Components/Material/MaterialComponentManager.h"

namespace JoeEngine {
    class JEEngineInstance {
    private:
        // Subsystems
        JESceneManager m_sceneManager;
        JEVulkanRenderer m_vulkanRenderer;
        JEIOHandler m_ioHandler;
        JEPhysicsManager m_physicsManager;

        // Entity-Component System Managers
        JEEntityManager m_entityManager;
        JEMeshComponentManager m_meshComponentManager;
        JEMaterialComponentManager m_materialComponentManager;

        double m_frameStartTime, m_frameEndTime; // timing for performance analysis
        bool m_enableFrameCounter;
        
        // Startup/shutdown
        void InitializeEngine();
        void StopEngine();

    public:
        JEEngineInstance() : m_enableFrameCounter(false), m_frameStartTime(0.0f), m_frameEndTime(0.0f) {
            InitializeEngine();
        }

        // Destroy resources/subsystems
        ~JEEngineInstance() {
            StopEngine();
        }

        // Run the main loop
        void Run();

        // Getters
        JEVulkanRenderer& GetRenderSubsystem() {
            return m_vulkanRenderer;
        }
        JEIOHandler& GetIOSubsystem() {
            return m_ioHandler;
        }
    };
}
