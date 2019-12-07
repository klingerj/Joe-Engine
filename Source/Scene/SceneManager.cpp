#include <chrono>

#include "SceneManager.h"
#include "../EngineInstance.h"
#include "../Components/Rotator/RotatorComponentManager.h"

namespace JoeEngine {
    void JESceneManager::Initialize(JEEngineInstance* engineInstance) {
        m_engineInstance = engineInstance;
    }

    void JESceneManager::LoadScene(uint32_t sceneId, VkExtent2D windowExtent, VkExtent2D shadowPassExtent) {
        m_currentScene = sceneId;
        if (sceneId == 0) {
            // TODO: this has to happen first or else bad things
            // Camera
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            std::vector<Entity> entities;
            MeshComponent meshComp_wahoo = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "wahoo.obj");
            MeshComponent meshComp_sphere = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "sphere.obj");
            MeshComponent meshComp_cube = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "cube.obj");

            uint32_t tex1 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "ducreux.jpg");
            uint32_t tex2 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Base_Color.jpg");
            uint32_t tex3 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Roughness.jpg");
            uint32_t tex4 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Metallic.jpg");
            uint32_t tex5 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Normal.jpg");
            uint32_t tex6 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "red.png");
            uint32_t tex7 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "blue.png");
            uint32_t tex8 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "orange.png");

            MaterialComponent mat_opaque_deferred;
            mat_opaque_deferred.m_geomType = TRIANGLES;
            mat_opaque_deferred.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred.m_renderLayer = OPAQUE + 1;
            mat_opaque_deferred.m_texAlbedo = tex2;
            m_engineInstance->CreateShader(mat_opaque_deferred, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new.spv");
            m_engineInstance->CreateDescriptor(mat_opaque_deferred);

            MaterialComponent mat_opaque_deferred2;
            mat_opaque_deferred2.m_geomType = TRIANGLES;
            mat_opaque_deferred2.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred2.m_renderLayer = OPAQUE + 2;
            mat_opaque_deferred2.m_texAlbedo = tex7;
            mat_opaque_deferred2.m_texNormal = tex5;
            mat_opaque_deferred2.m_shaderID = mat_opaque_deferred.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_opaque_deferred2);

            MaterialComponent mat_opaque_deferred3;
            mat_opaque_deferred3.m_geomType = TRIANGLES;
            mat_opaque_deferred3.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred3.m_renderLayer = OPAQUE;
            mat_opaque_deferred3.m_texAlbedo = tex8;
            mat_opaque_deferred3.m_texNormal = tex5;
            mat_opaque_deferred3.m_shaderID = mat_opaque_deferred.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_opaque_deferred3);

            MaterialComponent mat_opaque_deferred_noshadows;
            mat_opaque_deferred_noshadows.m_geomType = TRIANGLES;
            mat_opaque_deferred_noshadows.m_materialSettings = NO_SETTINGS;
            mat_opaque_deferred_noshadows.m_renderLayer = OPAQUE;
            mat_opaque_deferred_noshadows.m_texAlbedo = tex2;
            mat_opaque_deferred_noshadows.m_texNormal = tex5;
            m_engineInstance->CreateShader(mat_opaque_deferred_noshadows, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new_no_shadows.spv");
            m_engineInstance->CreateDescriptor(mat_opaque_deferred_noshadows);

            MaterialComponent mat_translucent_forward;
            mat_translucent_forward.m_geomType = TRIANGLES;
            mat_translucent_forward.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward.m_texAlbedo = tex6;
            m_engineInstance->CreateShader(mat_translucent_forward, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_no_shadows.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_forward);

            MaterialComponent mat_translucent_forward2;
            mat_translucent_forward2.m_geomType = TRIANGLES;
            mat_translucent_forward2.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward2.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward2.m_texAlbedo = tex7;
            mat_translucent_forward2.m_shaderID = mat_translucent_forward.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward2);

            MaterialComponent mat_translucent_forward3;
            mat_translucent_forward3.m_geomType = TRIANGLES;
            mat_translucent_forward3.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward3.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward3.m_texAlbedo = tex8;
            mat_translucent_forward3.m_shaderID = mat_translucent_forward.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward3);

            MaterialComponent mat_translucent_forward_noshadows;
            mat_translucent_forward_noshadows.m_geomType = TRIANGLES;
            mat_translucent_forward_noshadows.m_materialSettings = NO_SETTINGS;
            mat_translucent_forward_noshadows.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward_noshadows.m_texAlbedo = tex2;
            mat_translucent_forward_noshadows.m_texRoughness = tex3;
            mat_translucent_forward_noshadows.m_texMetallic = tex4;
            mat_translucent_forward_noshadows.m_texNormal = tex5;
            m_engineInstance->CreateShader(mat_translucent_forward_noshadows, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_no_shadows.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_forward_noshadows);

            MaterialComponent mat_translucent_forward_noshadows2;
            mat_translucent_forward_noshadows2.m_geomType = TRIANGLES;
            mat_translucent_forward_noshadows2.m_materialSettings = NO_SETTINGS;
            mat_translucent_forward_noshadows2.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward_noshadows2.m_texAlbedo = tex4;
            mat_translucent_forward_noshadows2.m_shaderID = mat_translucent_forward_noshadows.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward_noshadows2);

            for (int i = 0; i < 5; ++i) {
                for (int j = 0; j < 5; ++j) {
                    Entity newEntity = m_engineInstance->SpawnEntity();
                    entities.push_back(newEntity);
                    
                    if (i % 3 == 0) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_wahoo);
                    } else if (i % 3 == 1) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_sphere);
                    } else if (i % 3 == 2) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_cube);
                    }

                    if (i == 0) {
                        if (j <= 2) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward2);
                        }
                    } else if (i == 1) {
                        if (j <= 2) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);
                        } else if (j == 3) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred2);
                        } else if (j == 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred3);
                        }
                    } else if (i == 2) {
                        if (j == 0) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward2);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward3);
                        }
                    } else if (i == 3) {
                        m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred_noshadows);
                    } else if (i == 4) {
                        if (j <= 1) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward_noshadows);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward_noshadows2);
                        }
                    }

                    TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
                    trans->SetTranslation(glm::vec3(i - 32, 0, j - 32) * 0.1f);
                    trans->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));

                    m_engineInstance->AddComponent<RotatorComponent>(newEntity);
                    RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
                    rot->m_entityId = newEntity.GetId();
                    rot->m_axis = glm::vec3(0, 1, 0);
                }
            }

            // Ground plane
            Entity newEntity = m_engineInstance->SpawnEntity();
            entities.push_back(newEntity);

            MeshComponent meshComp_plane = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "plane.obj");
            m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_plane);

            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);

            TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
            trans->SetTranslation(glm::vec3(0.0f, -0.25f, 0.0f));
            trans->SetRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
            trans->SetScale(glm::vec3(12.0f, 12.0f, 1.0f));
            m_engineInstance->AddComponent<RotatorComponent>(newEntity);

            RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
            rot->m_entityId = newEntity.GetId();
        } else if (sceneId == 1) {
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            std::vector<Entity> entities;
            MeshComponent meshComp_wahoo = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "wahoo.obj");
            MeshComponent meshComp_sphere = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "sphere.obj");
            MeshComponent meshComp_cube = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "cube.obj");
            uint32_t tex1 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "ducreux.jpg");

            MaterialComponent mat_opaque_deferred;
            mat_opaque_deferred.m_geomType = TRIANGLES;
            mat_opaque_deferred.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred.m_renderLayer = OPAQUE;
            mat_opaque_deferred.m_texAlbedo = tex1;
            m_engineInstance->CreateShader(mat_opaque_deferred, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new.spv");
            m_engineInstance->CreateDescriptor(mat_opaque_deferred);

            for (int i = 0; i < 32; ++i) {
                for (int j = 0; j < 32; ++j) {
                    Entity newEntity = m_engineInstance->SpawnEntity();
                    entities.push_back(newEntity);

                    if (i % 3 == 0) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_wahoo);
                    } else if (i % 3 == 1) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_sphere);
                    } else if (i % 3 == 2) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_cube);
                    }

                    m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);

                    TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
                    trans->SetTranslation(glm::vec3(i - 32, 0, j - 32) * 0.1f);
                    trans->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));

                    m_engineInstance->AddComponent<RotatorComponent>(newEntity);
                    RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
                    rot->m_entityId = newEntity.GetId();
                    rot->m_axis = glm::vec3(0, 1, 0);
                }
            }

            // Ground plane
            Entity newEntity = m_engineInstance->SpawnEntity();
            entities.push_back(newEntity);

            MeshComponent meshComp_plane = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "plane.obj");
            m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_plane);

            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);

            TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
            trans->SetTranslation(glm::vec3(0.0f, -0.25f, 0.0f));
            trans->SetRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
            trans->SetScale(glm::vec3(12.0f, 12.0f, 1.0f));
            m_engineInstance->AddComponent<RotatorComponent>(newEntity);

            RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
            rot->m_entityId = newEntity.GetId();
        } else if (sceneId == 2) {
            m_camera = JECamera(glm::vec3(-3.5f, 2.1f, -3.5f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            std::vector<Entity> entities;
            MeshComponent meshComp_wahoo = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "wahoo.obj");
            MeshComponent meshComp_sphere = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "sphere.obj");
            MeshComponent meshComp_cube = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "cube.obj");

            uint32_t tex1 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "ducreux.jpg");
            uint32_t tex2 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Base_Color.jpg");
            uint32_t tex3 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Roughness.jpg");
            uint32_t tex4 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Metallic.jpg");
            uint32_t tex5 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "Metal_Plate_022a_Normal.jpg");
            uint32_t tex6 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "red.png");
            uint32_t tex7 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "blue.png");
            uint32_t tex8 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "orange.png");

            MaterialComponent mat_opaque_deferred;
            mat_opaque_deferred.m_geomType = TRIANGLES;
            mat_opaque_deferred.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred.m_renderLayer = OPAQUE + 1;
            mat_opaque_deferred.m_texAlbedo = tex2;
            m_engineInstance->CreateShader(mat_opaque_deferred, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new.spv");
            m_engineInstance->CreateDescriptor(mat_opaque_deferred);

            MaterialComponent mat_opaque_deferred2;
            mat_opaque_deferred2.m_geomType = TRIANGLES;
            mat_opaque_deferred2.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred2.m_renderLayer = OPAQUE + 2;
            mat_opaque_deferred2.m_texAlbedo = tex7;
            mat_opaque_deferred2.m_texNormal = tex5;
            mat_opaque_deferred2.m_shaderID = mat_opaque_deferred.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_opaque_deferred2);

            MaterialComponent mat_opaque_deferred3;
            mat_opaque_deferred3.m_geomType = TRIANGLES;
            mat_opaque_deferred3.m_materialSettings = ALL_SETTINGS;
            mat_opaque_deferred3.m_renderLayer = OPAQUE;
            mat_opaque_deferred3.m_texAlbedo = tex8;
            mat_opaque_deferred3.m_texNormal = tex5;
            mat_opaque_deferred3.m_shaderID = mat_opaque_deferred.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_opaque_deferred3);

            MaterialComponent mat_opaque_deferred_noshadows;
            mat_opaque_deferred_noshadows.m_geomType = TRIANGLES;
            mat_opaque_deferred_noshadows.m_materialSettings = NO_SETTINGS;
            mat_opaque_deferred_noshadows.m_renderLayer = OPAQUE;
            mat_opaque_deferred_noshadows.m_texAlbedo = tex2;
            mat_opaque_deferred_noshadows.m_texNormal = tex5;
            m_engineInstance->CreateShader(mat_opaque_deferred_noshadows, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new_no_shadows.spv");
            m_engineInstance->CreateDescriptor(mat_opaque_deferred_noshadows);

            MaterialComponent mat_translucent_forward;
            mat_translucent_forward.m_geomType = TRIANGLES;
            mat_translucent_forward.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward.m_texAlbedo = tex6;
            m_engineInstance->CreateShader(mat_translucent_forward, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_oit.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_forward);

            MaterialComponent mat_translucent_forward2;
            mat_translucent_forward2.m_geomType = TRIANGLES;
            mat_translucent_forward2.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward2.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward2.m_texAlbedo = tex7;
            mat_translucent_forward2.m_shaderID = mat_translucent_forward.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward2);

            MaterialComponent mat_translucent_forward3;
            mat_translucent_forward3.m_geomType = TRIANGLES;
            mat_translucent_forward3.m_materialSettings = CASTS_SHADOWS;
            mat_translucent_forward3.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward3.m_texAlbedo = tex8;
            mat_translucent_forward3.m_shaderID = mat_translucent_forward.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward3);

            MaterialComponent mat_translucent_forward_noshadows;
            mat_translucent_forward_noshadows.m_geomType = TRIANGLES;
            mat_translucent_forward_noshadows.m_materialSettings = NO_SETTINGS;
            mat_translucent_forward_noshadows.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward_noshadows.m_texAlbedo = tex2;
            mat_translucent_forward_noshadows.m_texRoughness = tex3;
            mat_translucent_forward_noshadows.m_texMetallic = tex4;
            mat_translucent_forward_noshadows.m_texNormal = tex5;
            m_engineInstance->CreateShader(mat_translucent_forward_noshadows, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_oit.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_forward_noshadows);

            MaterialComponent mat_translucent_forward_noshadows2;
            mat_translucent_forward_noshadows2.m_geomType = TRIANGLES;
            mat_translucent_forward_noshadows2.m_materialSettings = NO_SETTINGS;
            mat_translucent_forward_noshadows2.m_renderLayer = TRANSLUCENT;
            mat_translucent_forward_noshadows2.m_texAlbedo = tex4;
            mat_translucent_forward_noshadows2.m_shaderID = mat_translucent_forward_noshadows.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_forward_noshadows2);

            for (int i = 0; i < 5; ++i) {
                for (int j = 0; j < 5; ++j) {
                    Entity newEntity = m_engineInstance->SpawnEntity();
                    entities.push_back(newEntity);

                    if (i % 3 == 0) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_wahoo);
                    } else if (i % 3 == 1) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_sphere);
                    } else if (i % 3 == 2) {
                        m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_cube);
                    }

                    if (i == 0) {
                        if (j <= 2) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward2);
                        }
                    } else if (i == 1) {
                        if (j <= 2) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);
                        } else if (j == 3) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred2);
                        } else if (j == 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred3);
                        }
                    } else if (i == 2) {
                        if (j == 0) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward2);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward3);
                        }
                    } else if (i == 3) {
                        m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred_noshadows);
                    } else if (i == 4) {
                        if (j <= 1) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward_noshadows);
                        } else if (j <= 4) {
                            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_translucent_forward_noshadows2);
                        }
                    }

                    TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
                    trans->SetTranslation(glm::vec3(i - 32, 0, j - 32) * 0.1f);
                    trans->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));

                    m_engineInstance->AddComponent<RotatorComponent>(newEntity);
                    RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
                    rot->m_entityId = newEntity.GetId();
                    rot->m_axis = glm::vec3(0, 1, 0);
                }
            }

            // Ground plane
            Entity newEntity = m_engineInstance->SpawnEntity();
            entities.push_back(newEntity);

            MeshComponent meshComp_plane = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "plane.obj");
            m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_plane);

            m_engineInstance->SetComponent<JEMaterialComponentManager>(newEntity, mat_opaque_deferred);

            TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
            trans->SetTranslation(glm::vec3(0.0f, -0.25f, 0.0f));
            trans->SetRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
            trans->SetScale(glm::vec3(12.0f, 12.0f, 1.0f));
            m_engineInstance->AddComponent<RotatorComponent>(newEntity);

            RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
            rot->m_entityId = newEntity.GetId();

            // Particle System
            MaterialComponent particleMat;
            particleMat.m_geomType = POINTS;
            particleMat.m_materialSettings = NO_SETTINGS;
            particleMat.m_renderLayer = TRANSLUCENT;
            particleMat.m_texAlbedo = tex6;
            m_engineInstance->CreateShader(particleMat, JE_SHADER_DIR + "vert_points.spv", JE_SHADER_DIR + "frag_points.spv");
            m_engineInstance->CreateDescriptor(particleMat);
            m_engineInstance->InstantiateParticleSystem({ glm::vec3(0.0f, 1.0f, 0.0f), 1000.0f, 75000 }, particleMat);
            m_engineInstance->InstantiateParticleSystem({ glm::vec3(2.0f, 1.0f, 0.0f), 2000.0f, 75000 }, particleMat);
            m_engineInstance->InstantiateParticleSystem({ glm::vec3(4.0f, 1.0f, 0.0f), 3000.0f, 75000 }, particleMat);
            m_engineInstance->InstantiateParticleSystem({ glm::vec3(6.0f, 1.0f, 0.0f), 4000.0f, 75000 }, particleMat);
        } else if (sceneId == 3) {
            m_camera = JECamera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            MeshComponent meshComp_triLeft = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "triLeft.obj");
            MeshComponent meshComp_triRight = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "triRight.obj");

            uint32_t tex0 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "red.png");
            uint32_t tex1 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "blue.png");

            MaterialComponent mat_translucent_red;
            mat_translucent_red.m_geomType = TRIANGLES;
            mat_translucent_red.m_materialSettings = NO_SETTINGS;
            mat_translucent_red.m_renderLayer = TRANSLUCENT;
            mat_translucent_red.m_texAlbedo = tex0;
            m_engineInstance->CreateShader(mat_translucent_red, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_no_shadows.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_red);

            MaterialComponent mat_translucent_blue;
            mat_translucent_blue.m_geomType = TRIANGLES;
            mat_translucent_blue.m_materialSettings = NO_SETTINGS;
            mat_translucent_blue.m_renderLayer = TRANSLUCENT;
            mat_translucent_blue.m_texAlbedo = tex1;
            mat_translucent_blue.m_shaderID = mat_translucent_red.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_blue);

            Entity entity0 = m_engineInstance->SpawnEntity();
            m_engineInstance->SetComponent<JEMeshComponentManager>(entity0, meshComp_triLeft);
            m_engineInstance->SetComponent<JEMaterialComponentManager>(entity0, mat_translucent_red);

            Entity entity1 = m_engineInstance->SpawnEntity();
            m_engineInstance->SetComponent<JEMeshComponentManager>(entity1, meshComp_triRight);
            m_engineInstance->SetComponent<JEMaterialComponentManager>(entity1, mat_translucent_blue);
        } else if (sceneId == 4) {
            m_camera = JECamera(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            MeshComponent meshComp_triLeft = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "triLeft.obj");
            MeshComponent meshComp_triRight = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "triRight.obj");

            uint32_t tex0 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "red.png");
            uint32_t tex1 = m_engineInstance->LoadTexture(JE_TEXTURES_DIR + "blue.png");

            MaterialComponent mat_translucent_red;
            mat_translucent_red.m_geomType = TRIANGLES;
            mat_translucent_red.m_materialSettings = NO_SETTINGS;
            mat_translucent_red.m_renderLayer = TRANSLUCENT;
            mat_translucent_red.m_texAlbedo = tex0;
            m_engineInstance->CreateShader(mat_translucent_red, JE_SHADER_DIR + "vert_forward.spv", JE_SHADER_DIR + "frag_forward_new_oit.spv");
            m_engineInstance->CreateDescriptor(mat_translucent_red);

            MaterialComponent mat_translucent_blue;
            mat_translucent_blue.m_geomType = TRIANGLES;
            mat_translucent_blue.m_materialSettings = NO_SETTINGS;
            mat_translucent_blue.m_renderLayer = TRANSLUCENT;
            mat_translucent_blue.m_texAlbedo = tex1;
            mat_translucent_blue.m_shaderID = mat_translucent_red.m_shaderID;
            m_engineInstance->CreateDescriptor(mat_translucent_blue);

            Entity entity0 = m_engineInstance->SpawnEntity();
            m_engineInstance->SetComponent<JEMeshComponentManager>(entity0, meshComp_triLeft);
            m_engineInstance->SetComponent<JEMaterialComponentManager>(entity0, mat_translucent_red);

            Entity entity1 = m_engineInstance->SpawnEntity();
            m_engineInstance->SetComponent<JEMeshComponentManager>(entity1, meshComp_triRight);
            m_engineInstance->SetComponent<JEMaterialComponentManager>(entity1, mat_translucent_blue);
        }
    }

    void JESceneManager::RecreateResources(VkExtent2D windowExtent) {
        m_camera.SetAspect(windowExtent.width / (float)windowExtent.height);
    }

    void JESceneManager::RegisterCallbacks(JEIOHandler* ioHandler) {
        // Camera Movement
        JECallbackFunction cameraPanForward = [&] { m_camera.TranslateAlongLook(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanBackward = [&] { m_camera.TranslateAlongLook(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPanLeft = [&] { m_camera.TranslateAlongRight(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPanRight = [&] { m_camera.TranslateAlongRight(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanUp = [&] { m_camera.TranslateAlongUp(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanDown = [&] { m_camera.TranslateAlongUp(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPitchDown = [&] { m_camera.RotateAboutRight(-m_camRotateSensitivity); };
        JECallbackFunction cameraPitchUp = [&] { m_camera.RotateAboutRight(m_camRotateSensitivity); };
        JECallbackFunction cameraYawLeft = [&] { m_camera.RotateAboutUp(-m_camRotateSensitivity); };
        JECallbackFunction cameraYawRight = [&] { m_camera.RotateAboutUp(m_camRotateSensitivity); };
        ioHandler->AddCallback(JE_KEY_W, cameraPanForward);
        ioHandler->AddCallback(JE_KEY_A, cameraPanLeft);
        ioHandler->AddCallback(JE_KEY_S, cameraPanBackward);
        ioHandler->AddCallback(JE_KEY_D, cameraPanRight);
        ioHandler->AddCallback(JE_KEY_Q, cameraPanDown);
        ioHandler->AddCallback(JE_KEY_E, cameraPanUp);
        ioHandler->AddCallback(JE_KEY_UP, cameraPitchDown);
        ioHandler->AddCallback(JE_KEY_LEFT, cameraYawRight);
        ioHandler->AddCallback(JE_KEY_DOWN, cameraPitchUp);
        ioHandler->AddCallback(JE_KEY_RIGHT, cameraYawLeft);
    }
}
