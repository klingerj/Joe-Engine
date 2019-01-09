#include <chrono>

#include "SceneManager.h"

namespace JoeEngine {
    void JESceneManager::Initialize(const std::shared_ptr<JEMeshDataManager>& m) {
        m_meshDataManager = m;
    }

    void JESceneManager::LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEVulkanQueue& graphicsQueue, const JEVulkanSwapChain& vulkanSwapChain, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses, uint32_t sceneId) {
        m_currentScene = sceneId;
        if (sceneId == 0) {
            // Meshes
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_NONE);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_NONE);
            m_meshDataManager->SetMeshPosition(glm::vec3(0.0f, 0.0f, 0.0f), 0);
            m_meshDataManager->SetMeshPosition(glm::vec3(-0.5f, 3.0f, 0.0f), 1);
            m_meshDataManager->SetMeshPosition(glm::vec3(1.f, -1.0f, 0.0f), 2);
            m_meshDataManager->SetMeshPosition(glm::vec3(2.0f, -2.0f, 0.0f), 3);
            m_meshDataManager->SetMeshPosition(glm::vec3(3.0f, -2.0f, 0.0f), 4);
            m_meshDataManager->SetMeshPosition(glm::vec3(4.0f, -1.0f, 0.0f), 5);
            m_meshDataManager->SetMeshPosition(glm::vec3(5.0f, 0.0f, 0.0f), 6);
            m_meshDataManager->SetMeshPosition(glm::vec3(6.0f, 1.0f, 0.0f), 7);
            m_meshDataManager->SetMeshPosition(glm::vec3(7.0f, 2.0f, 0.0f), 8);
            m_meshDataManager->SetMeshPosition(glm::vec3(6.5f, 10.0f, 0.0f), 9);

            // Screen space triangle setup
            m_meshDataManager->CreateScreenSpaceTriangleMesh(physicalDevice, device, commandPool, graphicsQueue);

            // Textures
            JETexture t = JETexture(device, physicalDevice, graphicsQueue, commandPool, JE_TEXTURES_DIR + "ducreux.jpg");
            m_textures.push_back(t);

            // Camera
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPass.width / (float)shadowPass.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            // Shaders
            CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass_deferredLighting, deferredLightingImageView, shadowPass, deferredPass, postProcessingPasses);
        } else if (sceneId == 1) {
            // Meshes
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_NONE);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "cube.obj", JE_PHYSICS_FREEZE_NONE);
            m_meshDataManager->SetMeshPosition(glm::vec3(0.0f, 0.0f, 0.0f), 0);
            m_meshDataManager->SetMeshPosition(glm::vec3(-0.5f, 3.0f, 0.0f), 1);
            m_meshDataManager->SetMeshPosition(glm::vec3(1.f, -1.0f, 0.0f), 2);
            m_meshDataManager->SetMeshPosition(glm::vec3(0.75f, 3.0f, 0.0f), 3);

            // Screen space triangle setup
            m_meshDataManager->CreateScreenSpaceTriangleMesh(physicalDevice, device, commandPool, graphicsQueue);

            // Textures
            JETexture t = JETexture(device, physicalDevice, graphicsQueue, commandPool, JE_TEXTURES_DIR + "ducreux.jpg");
            m_textures.push_back(t);

            // Camera
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPass.width / (float)shadowPass.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            // Shaders
            CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass_deferredLighting, deferredLightingImageView, shadowPass, deferredPass, postProcessingPasses);
        } else if (sceneId == 2) {
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "plane.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "wahoo.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "sphere.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);
            m_meshDataManager->CreateNewMesh(physicalDevice, device, commandPool, graphicsQueue, JE_MODELS_OBJ_DIR + "alienModel_Small.obj", JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION);

            // Screen space triangle setup
            m_meshDataManager->CreateScreenSpaceTriangleMesh(physicalDevice, device, commandPool, graphicsQueue);

            // Textures
            JETexture t = JETexture(device, physicalDevice, graphicsQueue, commandPool, JE_TEXTURES_DIR + "ducreux.jpg");
            m_textures.push_back(t);

            // Camera
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPass.width / (float)shadowPass.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);

            // Shaders
            CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass_deferredLighting, deferredLightingImageView, shadowPass, deferredPass, postProcessingPasses);
        }
    }

    void JESceneManager::CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses) {
        m_shadowPassShaders.emplace_back(JEVulkanShadowPassShader(physicalDevice, device, shadowPass.renderPass, { static_cast<uint32_t>(shadowPass.width), static_cast<uint32_t>(shadowPass.height) }, m_meshDataManager->GetNumMeshes(),
            JE_SHADER_DIR + "vert_shadow.spv", JE_SHADER_DIR + "frag_shadow.spv"));
        m_deferredPassGeometryShader = JEVulkanDeferredPassGeometryShader(physicalDevice, device, vulkanSwapChain, deferredPass.renderPass, m_meshDataManager->GetNumMeshes(), m_textures[0],
            JE_SHADER_DIR + "vert_deferred_geom.spv", JE_SHADER_DIR + "frag_deferred_geom.spv");
        m_deferredPassLightingShader = JEVulkanDeferredPassLightingShader(physicalDevice, device, vulkanSwapChain, shadowPass, deferredPass, renderPass_deferredLighting, m_textures[0],
            JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting.spv");
        for (uint32_t p = 0; p < postProcessingPasses.size(); ++p) {
            JEPostProcessingPass& currentPass = postProcessingPasses[p];
            if (p == 0) {
                // Use the output of the deferred lighting pass as the input into the first post processing shader
                m_postProcessingShaders.emplace_back(JEVulkanPostProcessShader(physicalDevice, device, vulkanSwapChain, currentPass, deferredLightingImageView,
                    JE_SHADER_DIR + "vert_passthrough.spv", JE_SHADER_DIR + JEBuiltInPostProcessingShaderPaths[currentPass.shaderIndex]));
            } else {
                // Use the output of the previous post processing shader as the input into the first post processing shader
                m_postProcessingShaders.emplace_back(JEVulkanPostProcessShader(physicalDevice, device, vulkanSwapChain, postProcessingPasses[p], postProcessingPasses[p - 1].texture.imageView,
                    JE_SHADER_DIR + "vert_passthrough.spv", JE_SHADER_DIR + JEBuiltInPostProcessingShaderPaths[currentPass.shaderIndex]));
            }
        }
    }

    void JESceneManager::RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses) {
        CreateShaders(physicalDevice, device, vulkanSwapChain, renderPass_deferredLighting, deferredLightingImageView, shadowPass, deferredPass, postProcessingPasses);
        m_camera.SetAspect(vulkanSwapChain.GetExtent().width / (float)vulkanSwapChain.GetExtent().height);
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

    void JESceneManager::CleanupMeshesAndTextures(VkDevice device) {
        m_meshDataManager->Cleanup(device);
        for (JETexture t : m_textures) {
            t.Cleanup(device);
        }
        m_textures.clear();
    }

    void JESceneManager::CleanupShaders(VkDevice device) {
        for (auto& shadowPassShader : m_shadowPassShaders) {
            shadowPassShader.Cleanup(device);
        }
        m_deferredPassGeometryShader.Cleanup(device);
        m_deferredPassLightingShader.Cleanup(device);
        for (auto& postProcessingShader : m_postProcessingShaders) {
            postProcessingShader.Cleanup(device);
        }
        m_shadowPassShaders.clear();
        m_postProcessingShaders.clear();
    }

    void JESceneManager::UpdateModelMatrices() {
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        if (m_currentScene == 0) {

        } else if (m_currentScene == 1) {

        } else if (m_currentScene == 2) {
            glm::mat4 mat1 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
            mat1 = glm::rotate(mat1, -1.5708f, glm::vec3(1.0f, 0.0f, 0.0f));
            mat1 = glm::scale(mat1, glm::vec3(8.0f, 8.0f, 8.0f));
            m_meshDataManager->SetModelMatrix(mat1, 0);

            glm::mat4 mat2 = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.75f, 0.0f));
            mat2 = glm::rotate(mat2, 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            mat2 = glm::scale(mat2, glm::vec3(0.25f, 0.25f, 0.25f));
            m_meshDataManager->SetModelMatrix(mat2, 1);

            glm::mat4 mat3 = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, -0.75f, 0.05f));
            mat3 = glm::rotate(mat3, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
            mat3 = glm::scale(mat3, glm::vec3(0.15f, 0.15f, 0.15f));
            m_meshDataManager->SetModelMatrix(mat3, 2);

            glm::mat4 mat4 = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, std::sinf(time) * 0.05f - 0.75f, 0.75f));
            mat4 = glm::rotate(mat4, time * -1.5708f, glm::vec3(0.0f, 1.0f, 0.0f));
            mat4 = glm::scale(mat4, glm::vec3(0.15f, 0.15f, 0.15f));
            m_meshDataManager->SetModelMatrix(mat4, 3);
        }
    }

    void JESceneManager::UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex) {
        for (auto& shadowPassShader : m_shadowPassShaders) {
            shadowPassShader.UpdateUniformBuffers(device, m_shadowCamera, m_meshDataManager->GetModelMatrices(), m_meshDataManager->GetNumMeshes());
        }
        m_deferredPassGeometryShader.UpdateUniformBuffers(device, m_camera, m_meshDataManager->GetModelMatrices(), m_meshDataManager->GetNumMeshes());
        m_deferredPassLightingShader.UpdateUniformBuffers(device, imageIndex, m_camera, m_shadowCamera);
        for (auto& postProcessingShader : m_postProcessingShaders) {
            postProcessingShader.UpdateUniformBuffers(device, imageIndex, m_camera, m_shadowCamera, m_meshDataManager->GetModelMatrices(), m_meshDataManager->GetNumMeshes());
        }
    }

    void JESceneManager::BindShadowPassResources(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_shadowPassShaders[0].GetPipeline());
        for (uint32_t j = 0; j < m_meshDataManager->GetNumMeshes(); ++j) {
            uint32_t dynamicOffset = j * static_cast<uint32_t>(m_shadowPassShaders[0].GetDynamicAlignment());
            m_shadowPassShaders[0].BindDescriptorSets(commandBuffer, dynamicOffset);
            m_meshDataManager->DrawMesh(commandBuffer, j);
        }
    }

    void JESceneManager::BindDeferredPassGeometryResources(VkCommandBuffer commandBuffer) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredPassGeometryShader.GetPipeline());
        for (uint32_t j = 0; j < m_meshDataManager->GetNumMeshes(); ++j) {
            uint32_t dynamicOffset = j * static_cast<uint32_t>(m_deferredPassGeometryShader.GetDynamicAlignment());
            m_deferredPassGeometryShader.BindDescriptorSets(commandBuffer, dynamicOffset);
            m_meshDataManager->DrawMesh(commandBuffer, j);
        }
    }

    void JESceneManager::BindDeferredPassLightingResources(VkCommandBuffer commandBuffer, size_t index) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_deferredPassLightingShader.GetPipeline());
        m_deferredPassLightingShader.BindDescriptorSets(commandBuffer, index);
        m_meshDataManager->DrawScreenSpaceTriangle(commandBuffer);
    }

    void JESceneManager::BindPostProcessingPassResources(VkCommandBuffer commandBuffer, size_t index, size_t shaderIndex) {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_postProcessingShaders[shaderIndex].GetPipeline());
        m_postProcessingShaders[shaderIndex].BindDescriptorSets(commandBuffer, index);
        m_meshDataManager->DrawScreenSpaceTriangle(commandBuffer);
    }
}

