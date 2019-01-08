#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/VulkanShader.h"
#include "../rendering/VulkanRenderer.h"
#include "../io/IOHandler.h"
#include "MeshDataManager.h"

class JESceneManager {
private:
    // Camera(s)
    JECamera camera;
    JECamera shadowCamera;
    float camTranslateSensitivity, camRotateSensitivity;

    // Meshes
    std::shared_ptr<JEMeshDataManager> meshDataManager;

    // Textures
    std::vector<JETexture> textures;

    // Shaders
    std::vector<JEVulkanShadowPassShader> shadowPassShaders;
    JEVulkanDeferredPassGeometryShader deferredPassGeometryShader;
    JEVulkanDeferredPassLightingShader deferredPassLightingShader;
    std::vector<JEVulkanPostProcessShader> postProcessingShaders;

    // Scene IDs
    uint32_t currentScene;

public:
    JESceneManager() : camTranslateSensitivity(0.25f), camRotateSensitivity(0.05f), currentScene(0) {}
    ~JESceneManager() {}

    // Creation
    void Initialize(const std::shared_ptr<JEMeshDataManager>& p);
    void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass_firstPostProcess, VkImageView firstPostProcessImageView, const JEVulkanQueue& graphicsQueue, const JEVulkanSwapChain& vulkanSwapChain, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses, uint32_t sceneId);
    void CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses);
    void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, std::vector<JEPostProcessingPass> postProcessingPasses);

    // IO
    void RegisterCallbacks(JEIOHandler* ioHandler);
    
    // Cleanup
    void CleanupMeshesAndTextures(VkDevice device);
    void CleanupShaders(VkDevice device);

    // Updating of resources, called every frame
    void UpdateModelMatrices();
    void UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex);

    // Resource binding, called during command buffer generation
    void BindResources(VkCommandBuffer commandBuffer, size_t index);
    void BindShadowPassResources(VkCommandBuffer commandBuffer);
    void BindDeferredPassGeometryResources(VkCommandBuffer commandBuffer);
    void BindDeferredPassLightingResources(VkCommandBuffer commandBuffer, size_t index);
    void BindPostProcessingPassResources(VkCommandBuffer commandBuffer, size_t index, size_t shaderIndex);
};
