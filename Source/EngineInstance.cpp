#include <exception>
#include <memory>

#include "EngineInstance.h"

namespace JoeEngine {
    void JEEngineInstance::Run() {
        const JEVulkanWindow& window = m_vulkanRenderer.GetWindow();

        /*while (!window.ShouldClose()) {
            m_frameStartTime = glfwGetTime();
            m_ioHandler.PollInput();
            m_vulkanRenderer.SubmitFrame();
            m_physicsManager.Update();
            m_frameEndTime = glfwGetTime();
            if (m_enableFrameCounter) {
                std::cout << "Frame Time: " << (m_frameEndTime - m_frameStartTime) * 1000.0f << " ms" << std::endl;
            }
        }*/



        // TODO: ideal inner engine render loop for uniform binding - bind materials in good order

        while (!window.ShouldClose()) {
            // FPSCounter::StartFrame();

            m_ioHandler.PollInput(); // Receive input, call registered callback functions

            // Update components
            for (size_t i = 0; i < m_componentManagers.size(); ++i) {
                m_componentManagers[i]->Update();
            }

            // Submit shadow pass to GPU
            // TODO Don't cull before doing this? Also only send opaque geometry i think, not sure how to do transluscent shadows
            /*for (JELight l : m_sceneManager.Lights()) {
                m_renderer.ShadowPassStart();

                // bind the light frustum info
                // draw all opaque geometry

                m_renderer.ShadowPassEnd();
            }*/

            const std::vector<MeshComponent>& meshComponents = dynamic_cast<JEMeshComponentManager*>(m_componentManagers[MESH_COMP].get())->GetComponentList();

            m_vulkanRenderer.DrawShadowPass(meshComponents);
            //m_renderer.Submit();

            // Perform frustum culling
            // for each camera in the scene (probs one), do frustum culling
            /*for (JECamera cam : m_sceneManager.Cameras()) {
                if (cam.FrustumCullDirty() || m_entityManager.EntitiesDirty()) {
                    // if any game objects got destroyed or added, need to re-record 
                    // the command buffer for culling.
                    // * If I do a compute shader, then maybe the command buffer could stay the same and all I have 
                    // to do is pass the buffer of bounding boxes. Better allows for draw indirect
                    // * Could do on CPU to start out, use AVX probs, maybe multi-thread.

                    cam.Cull(m_boundingBoxComponentManager.GetBoundingBoxData(), m_meshComponentManager.RenderableMeshComponentIndices());
                    // TODO: if we want to use draw indirect eventually, then we need to convert these indices into the raw vert/index buffer
                    // handles. Then just call draw indirect on that below.

                    cam.FrustumCullClean();
                    m_entityManager.EntitiesClean();
                }
            }*/

            // At this point, m_meshComponentManager indices should contain the list of indices into its array of mesh components.
            // We should just be able to run through that list and access the other list to get all of our draw calls.

            //if (m_vulkanRenderer.RenderablesDirty()) { // true if any mesh component is dirty, e.g. camera moved, so diff stuff gets frustum culled

                //m_renderer.FrameStart();

                /*for (MeshComponent mc : m_meshComponentManager.GetComponentList()) {
                    mc.Draw();
                    // OR
                    m_vulkanRenderer.DrawMesh(mc);
                }*/
                
                m_vulkanRenderer.DrawMeshComponents(meshComponents);

                // TODO: This when culling is done
                /*for (uint32_t idx : m_meshComponentManager.RenderableMeshComponentIndices()) {
                    // bind material info via material component mgr
                    // TODO: need to order this loop optimally ^^^ sorting or something?
                    m_meshComponentManager.GetComponent(idx).Draw();
                    // OR
                    m_renderer.DrawMesh(m_meshComponentManager.GetComponent(idx), m_materialComponentManager.GetComponent(idx));
                }*/

                // TODO: that was for opaque. do the same for transluscent geometry now. Need 2 vectors in the mesh component manager.

                //m_renderer.FrameEnd();
            //}
            
            m_vulkanRenderer.SubmitFrame();

            // FPSCounter::EndFrame();
        }


        m_vulkanRenderer.WaitForIdleDevice();



        //vkDeviceWaitIdle(m_vulkanRenderer.GetDevice());
        StopEngine();
    }

    void JEEngineInstance::InitializeEngine() {
        // Init list of component managers
        // TODO: custom allocator instead of 'operator new'
        m_componentManagers.emplace_back(std::unique_ptr<JEMeshComponentManager>(new JEMeshComponentManager()));
        m_componentManagers.emplace_back(std::unique_ptr<JEMaterialComponentManager>(new JEMaterialComponentManager()));
        m_componentManagers.emplace_back(std::unique_ptr<JETransformComponentManager>(new JETransformComponentManager()));

        //m_physicsManager.Initialize(meshDataManager);
        m_sceneManager.Initialize(this);
        m_vulkanRenderer.Initialize(&m_sceneManager, this);

        GLFWwindow* window = m_vulkanRenderer.GetGLFWWindow();
        m_ioHandler.Initialize(window);
        glfwSetWindowUserPointer(window, this);
        m_vulkanRenderer.RegisterCallbacks(&m_ioHandler);
        m_sceneManager.RegisterCallbacks(&m_ioHandler);
    }

    void JEEngineInstance::StopEngine() {
        m_vulkanRenderer.Cleanup();
    }

    Entity JEEngineInstance::SpawnEntity() {
        Entity entity = m_entityManager.SpawnEntity();

        // Add new default-constructed components for the new entity
        for (size_t i = 0; i < m_componentManagers.size(); ++i) {
            m_componentManagers[i]->AddNewComponent();
        }

        return entity;
    }

    // User API

    MeshComponent JEEngineInstance::CreateMeshComponent(const std::string& filepath) {
        MeshComponent meshComp = m_vulkanRenderer.CreateMesh(filepath);
        return meshComp;
    }

    void JEEngineInstance::SetMeshComponent(const Entity& entity, const MeshComponent& meshComp) {
        dynamic_cast<JEMeshComponentManager*>(m_componentManagers[MESH_COMP].get())->SetComponent(entity.m_id, meshComp);
    }

    TransformComponent* JEEngineInstance::GetTransformComponent(const Entity& entity) {
        return dynamic_cast<JETransformComponentManager*>(m_componentManagers[TRANSFORM_COMP].get())->GetComponent(entity.m_id);
    }

    const std::vector<glm::mat4> JEEngineInstance::GetTransformMatrices() const {
        std::vector<glm::mat4> matrices;

        const std::vector<TransformComponent> transformComps = dynamic_cast<JETransformComponentManager*>(m_componentManagers[TRANSFORM_COMP].get())->GetComponentList();
        matrices.reserve(transformComps.size());

        for (TransformComponent t : transformComps) {
            matrices.emplace_back(t.GetTransform());
        }

        return matrices;
    }
}
