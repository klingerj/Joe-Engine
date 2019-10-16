#include <exception>
#include <memory>
#include <string>
//#include <utility>

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
                for (int i = 0; i < m_componentManagers.size(); ++i) {
                    {
                        //ScopedTimer<float> timer("Component Manager " + std::to_string(i));
                        m_componentManagers[i]->Update(this);
                    }
                }

                // Destroy any entities marked for deletion
                DestroyEntities();

                const PackedArray<MeshComponent>&      meshComponents      = GetComponentList<MeshComponent, JEMeshComponentManager>();
                const PackedArray<MaterialComponent>&  materialComponents  = GetComponentList<MaterialComponent, JEMaterialComponentManager>();
                const PackedArray<TransformComponent>& transformComponents = GetComponentList<TransformComponent, JETransformComponentManager>();

                // TODO: eventually get list of lights and pass those instead

                // TODO: scan/sort all material components so we only pass those that cast shadows to the shadow pass
                const std::vector<MaterialComponent>& materialComponentsVector = materialComponents.GetData();
                std::vector<std::pair<MaterialComponent, uint32_t>> indices;
                for (uint32_t i = 0; i < materialComponents.Size(); ++i) {
                    indices.emplace_back(std::pair<MaterialComponent, uint32_t>(materialComponentsVector[i], i));
                }

                std::sort(std::begin(indices), std::end(indices),
                    [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) -> bool {
                    return (a.first.m_materialSettings & CASTS_SHADOWS) > (b.first.m_materialSettings & CASTS_SHADOWS);
                });
                
                std::vector<MeshComponent>      meshComponentsSorted;
                std::vector<MaterialComponent>  materialComponentsSorted;
                std::vector<TransformComponent> transformComponentsSorted;

                for (uint32_t i = 0; i < indices.size(); ++i) {
                    if (!(indices[i].first.m_materialSettings & CASTS_SHADOWS)) {
                        break;
                    }
                    materialComponentsSorted.emplace_back(indices[i].first);
                    meshComponentsSorted.emplace_back(meshComponents.GetData()[indices[i].second]);
                    transformComponentsSorted.emplace_back(transformComponents.GetData()[indices[i].second]);
                }

                {
                    //ScopedTimer<float> timer("Shadow Pass Command Buffer Recording");
                    m_vulkanRenderer.DrawShadowPass(meshComponentsSorted, transformComponentsSorted, m_sceneManager.m_shadowCamera);
                }

                // Get bounding box info from MeshBuffer Manager
                const std::vector<BoundingBoxData>& boundingBoxes = m_vulkanRenderer.GetBoundingBoxData();

                std::vector<MeshComponent> meshComponentsPassedCulling;
                meshComponentsPassedCulling.reserve(64);
                std::vector<TransformComponent> transformComponentsPassedCulling;
                transformComponentsPassedCulling.reserve(64);

                {
                    //ScopedTimer<float> timer("Frustum Culling");

                    // TODO: multi-thread this
                    for (uint32_t i = 0; i < meshComponents.Size(); ++i) {
                        const MeshComponent& meshComp = meshComponents.GetData()[i];
                        if (meshComp.GetVertexHandle() == -1 || meshComp.GetIndexHandle() == -1) {
                            continue;
                        }

                        const TransformComponent& transformComp = transformComponents.GetData()[i];
                        if (m_sceneManager.m_camera.Cull(meshComp, transformComp, boundingBoxes[meshComp.GetVertexHandle()])) {
                            meshComponentsPassedCulling.emplace_back(meshComp);
                            transformComponentsPassedCulling.emplace_back(transformComp);
                        }
                    }
                }

                {
                    //ScopedTimer<float> timer("Deferred Geom/Lighting/Post Passes Command Buffer Recording");
                    m_vulkanRenderer.DrawMeshComponents(meshComponentsPassedCulling, transformComponentsPassedCulling, m_sceneManager.m_camera);
                }
                
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

    uint32_t JEEngineInstance::LoadTexture(const std::string& filepath) {
        return m_vulkanRenderer.CreateTexture(filepath);
    }

    void JEEngineInstance::RegisterMaterialComponent(MaterialComponent& materialComponent,
                                                     const std::string& vertFilepath, const std::string& fragFilepath) {
        // send the mat comp to the renderer, which will create a new shader with the shader mgr
        // that should return a handle to the shader mgr
        materialComponent.m_shaderID = m_vulkanRenderer.CreateShader(vertFilepath, fragFilepath);
    }
}
