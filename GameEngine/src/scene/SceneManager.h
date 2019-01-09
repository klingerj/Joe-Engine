#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/VulkanShader.h"
#include "../rendering/VulkanRenderer.h"
#include "../io/IOHandler.h"
#include "MeshDataManager.h"

namespace JoeEngine {
    class JESceneManager {
    private:
        // Camera(s)
        JECamera m_camera;
        JECamera m_shadowCamera;
        float m_camTranslateSensitivity, m_camRotateSensitivity;

        // Meshes
        ::std::shared_ptr<JEMeshDataManager> m_meshDataManager;

        // Textures
        ::std::vector<JETexture> m_textures;

        // Shaders
        ::std::vector<JEVulkanShadowPassShader> m_shadowPassShaders;
        JEVulkanDeferredPassGeometryShader m_deferredPassGeometryShader;
        JEVulkanDeferredPassLightingShader m_deferredPassLightingShader;
        ::std::vector<JEVulkanPostProcessShader> m_postProcessingShaders;

        // Scene IDs
        uint32_t m_currentScene;

    public:
        JESceneManager() : m_camTranslateSensitivity(0.25f), m_camRotateSensitivity(0.05f), m_currentScene(0) {}
        ~JESceneManager() {}

        // Creation
        void Initialize(const ::std::shared_ptr<JEMeshDataManager>& p);
        void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass_firstPostProcess, VkImageView firstPostProcessImageView, const JEVulkanQueue& graphicsQueue, const JEVulkanSwapChain& vulkanSwapChain, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, ::std::vector<JEPostProcessingPass> postProcessingPasses, uint32_t sceneId);
        void CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, ::std::vector<JEPostProcessingPass> postProcessingPasses);
        void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, ::std::vector<JEPostProcessingPass> postProcessingPasses);

        // IO
        void RegisterCallbacks(JEIOHandler* ioHandler);

        // Cleanup
        void CleanupMeshesAndTextures(VkDevice device);
        void CleanupShaders(VkDevice device);

        // Updating of resources, called every frame
        void UpdateModelMatrices();
        void UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex);

        // Resource binding, called during command buffer generation
        void BindShadowPassResources(VkCommandBuffer commandBuffer);
        void BindDeferredPassGeometryResources(VkCommandBuffer commandBuffer);
        void BindDeferredPassLightingResources(VkCommandBuffer commandBuffer, size_t index);
        void BindPostProcessingPassResources(VkCommandBuffer commandBuffer, size_t index, size_t shaderIndex);
    };
}
