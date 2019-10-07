#include <exception>
#include <memory>
#include <string>

#include "Utils/ScopedTimer.h"
#include "EngineInstance.h"

namespace JoeEngine {
    void JEEngineInstance::Run() {
        const JEVulkanWindow& window = m_vulkanRenderer.GetWindow();

        while (!window.ShouldClose()) {
            {
                //ScopedTimer<float> timer("Total Frame Time", "Frame start\n");
                const float startTime = glfwGetTime();

                {
                    //ScopedTimer<float> timer("Poll and Handle Input");
                    m_ioHandler.PollInput(); // Receive input, call registered callback functions
                }

                // Update components
                for (uint32_t i = 0; i < m_componentManagers.size(); ++i) {
                    {
                        //ScopedTimer<float> timer("Component Manager " + std::to_string(i));
                        m_componentManagers[i]->Update(this);
                    }
                }
                
                // Destroy any entities marked for deletion
                DestroyEntities();

                // Submit shadow pass to GPU
                // TODO Don't cull before doing this? Also only send opaque geometry i think, not sure how to do transluscent shadows
                /*for (JELight l : m_sceneManager.Lights()) {
                    m_renderer.ShadowPassStart();

                    // bind the light frustum info
                    // draw all opaque geometry

                    m_renderer.ShadowPassEnd();
                }*/

                const PackedArray<MeshComponent>&      meshComponents =      GetComponentList<MeshComponent,      JEMeshComponentManager>();
                const PackedArray<TransformComponent>& transformComponents = GetComponentList<TransformComponent, JETransformComponentManager>();

                // TODO: eventually get list of lights and pass those instead

                {
                    //ScopedTimer<float> timer("Shadow Pass Command Buffer Recording");
                    m_vulkanRenderer.DrawShadowPass(meshComponents, transformComponents, m_sceneManager.m_shadowCamera);
                }
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

                {
                    //ScopedTimer<float> timer("Deferred Geom/Lighting/Post Passes Command Buffer Recording");
                    m_vulkanRenderer.DrawMeshComponents(meshComponents, transformComponents, m_sceneManager.m_camera);
                }

                    // TODO: This when culling is done
                    /*for (uint32_t idx : m_meshComponentManager.RenderableMeshComponentIndices()) {
                        // bind material info via material component mgr
                        // TODO: need to order this loop optimally ^^^ sorting or something?
                        m_meshComponentManager.GetComponent(idx).Draw();
                        // OR
                        m_renderer.DrawMesh(m_meshComponentManager.GetComponent(idx), m_materialComponentManager.GetComponent(idx));
                    }*/

                    // TODO: that was for opaque. do the same for transluscent geometry now. Need 2 vectors in the mesh component manager.
                //}
                
                {
                    //ScopedTimer<float> timer("GPU workload submission");
                    m_vulkanRenderer.SubmitFrame();
                }

                // TODO: clean this up?
                const float endTime = glfwGetTime();
                const float elapsedTime = endTime - startTime;
                const float fps = 1.0f / elapsedTime;
                static float avgElapsed = 0.0f;
                static float avgFps = 0.0f;
                static uint32_t numFrames = 0;
                ++numFrames;
                avgElapsed += elapsedTime;
                avgFps += fps;
                glfwSetWindowTitle(m_vulkanRenderer.GetGLFWWindow(), ("The Joe Engine - Demo - " +
                                                                      std::to_string(elapsedTime * 1000.0f) + " ms / frame, " +
                                                                      std::to_string(avgElapsed * 1000.0f / numFrames).c_str() + " avg ms / frame | " +
                                                                      std::to_string(fps) + " fps, " + 
                                                                      std::to_string(avgFps / (float)numFrames).c_str() + " avg fps").c_str());
            }
        }

        m_vulkanRenderer.WaitForIdleDevice();
        StopEngine();
    }

    void JEEngineInstance::InitializeEngine() {
        {
            ScopedTimer<float> timer("Initialize Joe Engine");
            // Init list of component managers
            // TODO: custom allocator instead of 'operator new'
            RegisterComponentManager<MeshComponent>(new JEMeshComponentManager());
            RegisterComponentManager<MaterialComponent>(new JEMaterialComponentManager());
            RegisterComponentManager<TransformComponent>(new JETransformComponentManager());

            //m_physicsManager.Initialize(meshDataManager);
            m_sceneManager.Initialize(this);
            m_vulkanRenderer.Initialize(&m_sceneManager, this);

            GLFWwindow* window = m_vulkanRenderer.GetGLFWWindow();
            m_ioHandler.Initialize(window);
            glfwSetWindowUserPointer(window, this);
            m_vulkanRenderer.RegisterCallbacks(&m_ioHandler);
            m_sceneManager.RegisterCallbacks(&m_ioHandler);
        }
    }

    void JEEngineInstance::StopEngine() {
        m_vulkanRenderer.Cleanup();
    }

    void JEEngineInstance::DestroyEntities() {
        for (Entity e : m_destroyedEntities) {
            for (uint32_t i = 0; i < m_componentManagers.size(); ++i) {
                m_componentManagers[i]->RemoveComponent(e.GetId());
            }
            m_entityManager.DestroyEntity(e);
        }
        m_destroyedEntities.clear();
    }

    // User API

    Entity JEEngineInstance::SpawnEntity() {
        Entity entity = m_entityManager.SpawnEntity();

        // Add new default-constructed components for the new entity
        // Any new entity only gets a mesh, material, and transform, hence the 3
        // TODO: get rid of the hard-coded value
        for (uint32_t i = 0; i < 3 /*m_componentManagers.size()*/; ++i) {
            m_componentManagers[i]->AddNewComponent(entity.GetId());
        }

        return entity;
    }

    void JEEngineInstance::DestroyEntity(Entity entity) {
        m_destroyedEntities.push_back(entity);
    }

    void JEEngineInstance::LoadScene(uint32_t id) {
        m_sceneManager.LoadScene(id, { JE_DEFAULT_SCREEN_WIDTH, JE_DEFAULT_SCREEN_HEIGHT },
                                     { JE_DEFAULT_SHADOW_MAP_WIDTH, JE_DEFAULT_SHADOW_MAP_HEIGHT });
    }

    MeshComponent JEEngineInstance::CreateMeshComponent(const std::string& filepath) {
        MeshComponent meshComp = m_vulkanRenderer.CreateMesh(filepath);
        return meshComp;
    }
}
