#pragma once

#include <string>

#include "Io/IOHandler.h"
#include "Scene/SceneManager.h"
#include "Rendering/VulkanRenderer.h"
#include "Physics/PhysicsManager.h"
#include "Scene/EntityManager.h"
#include "Components/ComponentManager.h"
#include "Components/Mesh/MeshComponentManager.h"
#include "Components/Material/MaterialComponentManager.h"
#include "Components/Transform/TransformComponentManager.h"

namespace JoeEngine {
    class JEEngineInstance {
    private:
        // Subsystems
        JESceneManager m_sceneManager;
        JEVulkanRenderer m_vulkanRenderer;
        JEIOHandler m_ioHandler;
        //JEPhysicsManager m_physicsManager;

        // Entity-Component System Managers
        JEEntityManager m_entityManager;

        enum JE_COMP_MGR_IDX : uint8_t {
            MESH_COMP = 0,
            MATERIAL_COMP = 1,
            TRANSFORM_COMP = 2
            // TODO: more
        };

        std::vector<std::unique_ptr<JEComponentManager>> m_componentManagers;
        //JEMeshComponentManager m_meshComponentManager;
        //JEMaterialComponentManager m_materialComponentManager;

        double m_frameStartTime, m_frameEndTime; // timing for performance analysis
        bool m_enableFrameCounter;
        
        // Startup/shutdown
        void InitializeEngine();
        void StopEngine();

    public:
        JEEngineInstance() : m_enableFrameCounter(false), m_frameStartTime(0.0f), m_frameEndTime(0.0f) {
            InitializeEngine();
        }
        ~JEEngineInstance() {}

        // Run the main loop
        void Run();

        // Getters
        JEVulkanRenderer& GetRenderSubsystem() {
            return m_vulkanRenderer;
        }
        JEIOHandler& GetIOSubsystem() {
            return m_ioHandler;
        }

        // User API
        Entity SpawnEntity();
        MeshComponent CreateMeshComponent(const std::string& filepath);
        void SetMeshComponent(const Entity& entity, const MeshComponent& meshComp);
        TransformComponent* GetTransformComponent(const Entity& entity);
        const std::vector<glm::mat4> GetTransformMatrices() const;
    };
}
