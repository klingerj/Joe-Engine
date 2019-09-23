#pragma once

//#include <vector>

#include "Camera.h"
#include "../Io/IOHandler.h"

namespace JoeEngine {
    class JEEngineInstance;

    class JESceneManager {
    private:
        // Camera(s)
        
        float m_camTranslateSensitivity, m_camRotateSensitivity;

        // Meshes
        //std::shared_ptr<JEMeshDataManager> m_meshDataManager;

        // Textures
        //std::vector<JETexture> m_textures;

        // Shaders
        /*std::vector<JEVulkanShadowPassShader> m_shadowPassShaders;
        JEVulkanDeferredPassGeometryShader m_deferredPassGeometryShader;
        JEVulkanDeferredPassLightingShader m_deferredPassLightingShader;
        std::vector<JEVulkanPostProcessShader> m_postProcessingShaders;*/

        

        // Scene IDs
        uint32_t m_currentScene;

        JEEngineInstance* m_engineInstance;

    public:
        JESceneManager() : m_camTranslateSensitivity(0.25f), m_camRotateSensitivity(0.05f), m_currentScene(0), m_engineInstance(nullptr) {}
        ~JESceneManager() {}

        // TODO: make these private again
        JECamera m_camera;
        JECamera m_shadowCamera;

        // Creation
        void Initialize(JEEngineInstance* engineInstance);
        void LoadScene(uint32_t sceneId, VkExtent2D windowExtent, VkExtent2D shadowPassExtent);
        void RecreateResources(VkExtent2D windowExtent);

        // IO
        void RegisterCallbacks(JEIOHandler* ioHandler);

        // Cleanup
        //void CleanupMeshesAndTextures(VkDevice device);
        //void CleanupShaders(VkDevice device);

        // Updating of resources, called every frame
        //void UpdateModelMatrices();
        //void UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex);

        // Resource binding, called during command buffer generation
        //void BindShadowPassResources(VkCommandBuffer commandBuffer);
        //void BindDeferredPassGeometryResources(VkCommandBuffer commandBuffer);
        //void BindDeferredPassLightingResources(VkCommandBuffer commandBuffer, size_t index);
        //void BindPostProcessingPassResources(VkCommandBuffer commandBuffer, size_t index, size_t shaderIndex);
    };
}
