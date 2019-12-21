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
    //! The Engine Instance class.
    /*!
      This is the most important class - the actual instance of the Joe Engine. It owns and manages all the major subsystems,
      such as rendering, entities and components, resource loading, and scene management.
      It also has many useful API functions for the user to invoke.
    */
    class JEEngineInstance {
    private:
        // Major subsystems

        //! Scene manager.
        JESceneManager m_sceneManager;

        //! Renderer.
        JEVulkanRenderer m_vulkanRenderer;

        //! IO Handler.
        JEIOHandler m_ioHandler;

        //! Physics manager.
        JEPhysicsManager m_physicsManager;

        // Entity manager.
        JEEntityManager m_entityManager;

        //! List of various component managers.
        std::vector<std::unique_ptr<JEComponentManager>> m_componentManagers;
        
        //! List of particle systems.
        std::vector<JEParticleSystem> m_particleSystems;

        //! Map of component type to index in the list 'm_componentManagers'.
        // TODO: replace me?
        std::unordered_map<std::type_index, uint32_t> m_componentTypeToIndex;
        
        //! Initialization/startup function.
        void InitializeEngine(RendererSettings rendererSettings);

        //! Stop/shutdown function.
        void StopEngine();

        //! List of entities destroyed this frame.
        std::vector<Entity> m_destroyedEntities;

        //! Synchronously destroy entities in the list 'm_destroyedEntities', then clear the list.
        void DestroyEntities();

    public:
        // Default constructor.
        /*! Invokes the other constructor with default settings. */
        JEEngineInstance() : JEEngineInstance(RendererSettings::Default) {}

        //! Constructor.
        /*! Invokes the initialization function. */
        JEEngineInstance(RendererSettings rendererSettings) {
            InitializeEngine(rendererSettings);
        }

        //! Destructor (default).
        ~JEEngineInstance() = default;

        //! Run the main loop.
        void Run();

        //! Get the renderer subsystem.
        JEVulkanRenderer& GetRenderSubsystem() {
            return m_vulkanRenderer;
        }

        //! Get the IO subsystem.
        JEIOHandler& GetIOSubsystem() {
            return m_ioHandler;
        }

        //! User function - spawn an entity into the scene.
        Entity SpawnEntity();

        //! User function - destroy an entity in the scene.
        void DestroyEntity(Entity entity);

        //! Load a particular scene.
        void LoadScene(uint32_t id);

        //! Register a component manager with the engine.
        template <typename T, typename U>
        void RegisterComponentManager() {
            // TODO: custom allocator instead of 'operator new'?
            m_componentManagers.emplace_back(std::unique_ptr<U>(new U()));
            m_componentTypeToIndex[typeid(T)] = m_componentManagers.size() - 1;
        }

        //! Get a particular component manager's list of components.
        template <typename T, typename U>
        const PackedArray<T>& GetComponentList() const {
            return static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->GetComponentList();
        }

        //! Create a mesh component with a specific path to a mesh file. Invokes a function in the renderer.
        // TODO: replace me with something more general? or not, as this is a built-in component type
        MeshComponent CreateMeshComponent(const std::string& filepath);

        //! Load a texture into the engine at a specific path. Invokes a function in the renderer.
        uint32_t LoadTexture(const std::string& filepath);

        //! Load a shader into the engine at a specific vertex/fragment filepath pair and store in the material component.
        //! Invokes a function in the renderer.
        void CreateShader(MaterialComponent& materialComponent, const std::string& vertFilepath, const std::string& fragFilepath);

        //! Create a descriptor from the specified material component properties. Invokes a function in the renderer.
        void CreateDescriptor(MaterialComponent& materialComponent);

        //! Instantiate a particle system with specific settings in the scene.
        void InstantiateParticleSystem(const JEParticleSystemSettings& settings, const MaterialComponent& materialComponent);

        //! Add a component to a particular entity.
        template<typename T>
        void AddComponent(const Entity& entity) {
            m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get()->AddNewComponent(entity.GetId());
        }

        //! Get the component attached to an entity.
        template <typename T, typename U>
        T* GetComponent(const Entity& entity) const {
            return static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->GetComponent(entity.GetId());
        }

        //! Set a entity's component data to some new component data.
        template <typename U, typename T>
        void SetComponent(const Entity& entity, const T& comp) {
            static_cast<U*>(m_componentManagers[m_componentTypeToIndex.at(typeid(T))].get())->SetComponent(entity.GetId(), comp);
        }
    };
}
