#pragma once

#include <string>
#include <unordered_map>
#include <typeindex>

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

        std::vector<std::unique_ptr<JEComponentManager>> m_componentManagers;

        // TODO: replace me?
        std::unordered_map<std::type_index, uint32_t> m_componentTypeToIndex;
        
        // Startup/shutdown
        void InitializeEngine();
        void StopEngine();

        template <typename T, typename U>
        const PackedArray<T>& GetComponentList() const {
            return static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->GetComponentList();
        }

        std::vector<Entity> m_destroyedEntities;
        void DestroyEntities();

        /*template <typename T, typename U>
        const std::unique_ptr<U> GetComponentManager() const {
            return static_cast<std::unique_ptr<U>>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))]);
            //return m_componentManagers[m_componentTypeToIndex.find(typeid(T))]];
        }*/

    public:
        JEEngineInstance() {
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
        void DestroyEntity(Entity entity);

        // Load a scene
        void LoadScene(uint32_t id);

        template <typename T, typename U>
        void RegisterComponentManager(U* componentMgr) {
            m_componentManagers.emplace_back(std::unique_ptr<U>(componentMgr));
            m_componentTypeToIndex[typeid(T)] = m_componentManagers.size() - 1;
        }

        // TODO: replace me with something more general? or not, as this is a built-in component type
        MeshComponent CreateMeshComponent(const std::string& filepath);

        uint32_t LoadTexture(const std::string& filepath);
        void RegisterMaterialComponent(MaterialComponent& materialComponent, const std::string& vertFilepath, const std::string& fragFilepath);

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
