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

                // Update particle systems
                {
                    ScopedTimer<float> timer("Update particle systems");
                    m_physicsManager.UpdateParticleSystems(m_particleSystems);
                }

                {
                    ScopedTimer<float> timer("Update particle systems meshes");
                    for (uint32_t i = 0; i < m_particleSystems.size(); ++i) {
                        JEParticleSystem& particleSystem = m_particleSystems[i];
                        m_vulkanRenderer.UpdateMesh(particleSystem.m_meshComponent, particleSystem.GetVertices(), particleSystem.GetIndices());
                    }
                }
                
                const PackedArray<MeshComponent>&      meshComponents      = GetComponentList<MeshComponent, JEMeshComponentManager>();
                const PackedArray<MaterialComponent>&  materialComponents  = GetComponentList<MaterialComponent, JEMaterialComponentManager>();
                const PackedArray<TransformComponent>& transformComponents = GetComponentList<TransformComponent, JETransformComponentManager>();

                // TODO: eventually get list of lights and pass those instead

                // TODO: scan/sort all material components so we only pass those that cast shadows to the shadow pass
                const std::vector<MaterialComponent> materialComponentsVector = std::vector(materialComponents.begin(), materialComponents.end());
                std::vector<std::pair<MaterialComponent, uint32_t>> indices;
                for (uint32_t i = 0; i < materialComponentsVector.size(); ++i) {
                    indices.emplace_back(std::pair<MaterialComponent, uint32_t>(materialComponentsVector[i], i));
                }

                // Sort by material settings - only send shadow-casting geometry to the shadow pass
                std::sort(std::begin(indices), std::end(indices),
                    [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                    return (a.first.m_materialSettings & CASTS_SHADOWS) > (b.first.m_materialSettings & CASTS_SHADOWS);
                });

                uint32_t k = 0;
                for (k = 0; k < indices.size(); ++k) {
                    if (!(indices[k].first.m_materialSettings & CASTS_SHADOWS)) {
                        break;
                    }
                }
                
                std::vector<MeshComponent>      meshComponentsSorted_shadow;
                std::vector<MaterialComponent>  materialComponentsSorted_shadow;
                std::vector<glm::mat4> transformComponentsSorted_shadow;
                meshComponentsSorted_shadow.reserve(k);
                materialComponentsSorted_shadow.reserve(k);
                transformComponentsSorted_shadow.reserve(k);

                std::sort(indices.begin(), indices.begin() + k,
                    [&](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) -> bool {
                    return (meshComponents[a.second].GetVertexHandle()) < (meshComponents[b.second].GetVertexHandle());
                });

                for (uint32_t j = 0; j < k; ++j) {
                    meshComponentsSorted_shadow.emplace_back(meshComponents.GetData()[indices[j].second]);
                    transformComponentsSorted_shadow.emplace_back(transformComponents.GetData()[indices[j].second].GetTransform());
                }

                m_vulkanRenderer.StartFrame();

                {
                    //ScopedTimer<float> timer("Shadow Pass Command Buffer Recording");
                    m_vulkanRenderer.DrawShadowPass(meshComponentsSorted_shadow, m_sceneManager.m_shadowCamera);
                }

                // Get bounding box info from MeshBuffer Manager
                const std::vector<BoundingBoxData>& boundingBoxes = m_vulkanRenderer.GetBoundingBoxData();

                std::vector<MeshComponent> meshComponentsPassedCulling;
                std::vector<MaterialComponent> materialComponentsPassedCulling;
                std::vector<glm::mat4> transformsPassedCulling;
                meshComponentsPassedCulling.reserve(256);
                materialComponentsPassedCulling.reserve(256);
                transformsPassedCulling.reserve(256);

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
                            transformsPassedCulling.emplace_back(transformComp.GetTransform());
                            materialComponentsPassedCulling.emplace_back(materialComponents.GetData()[i]);
                        }
                    }
                }

                // TODO: sort materials by render layer, material (shader index and source textures), and mesh
                // the renderer will attempt to render them as instanced geometry and will minimize pipeline/descriptor binding
                // Sort all components by material properties and by mesh for efficient descriptor set binding and instanced rendering
                indices.clear();
                std::vector<MeshComponent>      meshComponentsSorted;
                std::vector<MaterialComponent>  materialComponentsSorted;
                std::vector<glm::mat4> transformComponentsSorted;
                for (uint32_t i = 0; i < materialComponentsPassedCulling.size(); ++i) {
                    indices.emplace_back(std::pair<MaterialComponent, uint32_t>(materialComponentsPassedCulling[i], i));
                }

                // 1. Sort by render layer
                std::sort(std::begin(indices), std::end(indices),
                    [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                    return (a.first.m_renderLayer) < (b.first.m_renderLayer);
                });

                // 2. Sort by shader index
                uint32_t idx = 0;
                uint32_t currSortIdx = 0;
                while (idx <= indices.size()) {
                    if (idx == indices.size()) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                            return (a.first.m_shaderID) < (b.first.m_shaderID);
                        });
                        break;
                    }

                    // Detect a change in the sorted materials
                    if (indices[idx].first.m_renderLayer != indices[currSortIdx].first.m_renderLayer) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                            return (a.first.m_shaderID) < (b.first.m_shaderID);
                        });
                        currSortIdx = idx;
                    }
                    ++idx;
                }

                // 3. Sort by descriptor index
                idx = 0;
                currSortIdx = 0;
                while (idx <= indices.size()) {
                    if (idx == indices.size()) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                            return (a.first.m_descriptorID) < (b.first.m_descriptorID);
                        });
                        break;
                    }

                    // Detect a change in the sorted materials
                    if (indices[idx].first.m_shaderID != indices[currSortIdx].first.m_shaderID) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) {
                            return (a.first.m_descriptorID) < (b.first.m_descriptorID);
                        });
                        currSortIdx = idx;
                    }
                    ++idx;
                }

                // 4. Sort by mesh component (needed for instanced rendering)
                idx = 0;
                currSortIdx = 0;
                while (idx <= indices.size()) {
                    if (idx == indices.size()) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [&](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) -> bool {
                            return (meshComponentsPassedCulling[a.second].GetVertexHandle()) < (meshComponentsPassedCulling[b.second].GetVertexHandle());
                        });
                        break;
                    }

                    // Detect a change in the sorted materials
                    if (indices[idx].first.m_descriptorID != indices[currSortIdx].first.m_descriptorID) {
                        std::sort(indices.begin() + currSortIdx, indices.begin() + idx,
                            [&](const std::pair<MaterialComponent, uint32_t>& a, const std::pair<MaterialComponent, uint32_t>& b) -> bool {
                            return (meshComponentsPassedCulling[a.second].GetVertexHandle()) < (meshComponentsPassedCulling[b.second].GetVertexHandle());
                        });
                        currSortIdx = idx;
                    }
                    ++idx;
                }

                for (uint32_t i = 0; i < indices.size(); ++i) {
                    materialComponentsSorted.emplace_back(indices[i].first);
                    meshComponentsSorted.emplace_back(meshComponentsPassedCulling[indices[i].second]);
                    transformComponentsSorted.emplace_back(transformsPassedCulling[indices[i].second]);
                }

                {
                    //ScopedTimer<float> timer("Deferred Geom/Lighting/Post Passes Command Buffer Recording");
                    m_vulkanRenderer.DrawMeshes(meshComponentsSorted, materialComponentsSorted, m_sceneManager.m_camera, m_particleSystems);
                }
                
                {
                    //ScopedTimer<float> timer("GPU workload submission");
                    m_vulkanRenderer.SubmitFrame(materialComponentsSorted, transformComponentsSorted_shadow, transformComponentsSorted);
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

    void JEEngineInstance::InitializeEngine(RendererSettings rendererSettings) {
        {
            ScopedTimer<float> timer("Initialize Joe Engine");
            // Init list of component managers
            RegisterComponentManager<MeshComponent, JEMeshComponentManager>();
            RegisterComponentManager<MaterialComponent, JEMaterialComponentManager>();
            RegisterComponentManager<TransformComponent, JETransformComponentManager>();

            m_physicsManager.Initialize();
            m_sceneManager.Initialize(this);
            m_vulkanRenderer.Initialize(rendererSettings, &m_sceneManager, this);

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

    void JEEngineInstance::CreateShader(MaterialComponent& materialComponent,
                                                     const std::string& vertFilepath, const std::string& fragFilepath) {
        m_vulkanRenderer.CreateShader(materialComponent, vertFilepath, fragFilepath);
    }

    void JEEngineInstance::CreateDescriptor(MaterialComponent& materialComponent) {
        uint32_t descrID = m_vulkanRenderer.CreateDescriptor(materialComponent);
        materialComponent.m_descriptorID = descrID;
    }

    void JEEngineInstance::InstantiateParticleSystem(const JEParticleSystemSettings& settings, const MaterialComponent& materialComponent) {
        m_particleSystems.emplace_back(JEParticleSystem(settings));

        JEParticleSystem& particleSystem = m_particleSystems[m_particleSystems.size() - 1];
        particleSystem.m_meshComponent = m_vulkanRenderer.m_meshBufferManager.CreateMeshComponent(particleSystem.GetVertices(),
                                                                 particleSystem.GetIndices());
        particleSystem.m_materialComponent = materialComponent;
    }
}
