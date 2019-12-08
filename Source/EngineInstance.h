#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

#include "Io/IOHandler.h"
#include "Scene/SceneManager.h"
#include "Rendering/VulkanRenderer.h"
#include "Physics/ParticleSystem.h"
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
        JEPhysicsManager m_physicsManager;

        // Entity-Component System Managers
        JEEntityManager m_entityManager;

        std::vector<std::unique_ptr<JEComponentManager>> m_componentManagers;

        std::vector<JEParticleSystem> m_particleSystems;

        // TODO: replace me?
        std::unordered_map<std::type_index, uint32_t> m_componentTypeToIndex;
        
        // Startup/shutdown
        void InitializeEngine(RendererSettings rendererSettings);
        void StopEngine();

        std::vector<Entity> m_destroyedEntities;
        void DestroyEntities();

    public:
        JEEngineInstance() : JEEngineInstance(RendererSettings::Default) {}
        JEEngineInstance(RendererSettings rendererSettings) {
            InitializeEngine(rendererSettings);
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
        void DestroyEntity(Entity entity);

        // Load a scene
        void LoadScene(uint32_t id);

        template <typename T, typename U>
        void RegisterComponentManager() {
            // TODO: custom allocator instead of 'operator new'?
            m_componentManagers.emplace_back(std::unique_ptr<U>(new U()));
            m_componentTypeToIndex[typeid(T)] = m_componentManagers.size() - 1;
        }

        template <typename T, typename U>
        const PackedArray<T>& GetComponentList() const {
            return static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->GetComponentList();
        }

        // TODO: replace me with something more general? or not, as this is a built-in component type
        MeshComponent CreateMeshComponent(const std::string& filepath);

        uint32_t LoadTexture(const std::string& filepath);
        void CreateShader(MaterialComponent& materialComponent, const std::string& vertFilepath, const std::string& fragFilepath);
        void CreateDescriptor(MaterialComponent& materialComponent);

        void InstantiateParticleSystem(const JEParticleSystemSettings& settings, const MaterialComponent& materialComponent);

        template<typename T>
        void AddComponent(const Entity& entity) {
            m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get()->AddNewComponent(entity.GetId());
        }

        template <typename T, typename U>
        T* GetComponent(const Entity& entity) const {
            return static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->GetComponent(entity.GetId());
        }

        template <typename U, typename T>
        void SetComponent(const Entity& entity, const T& comp) {
            static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->SetComponent(entity.GetId(), comp);
        }
    };
}
