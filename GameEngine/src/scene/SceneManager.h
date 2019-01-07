#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/VulkanShader.h"
#include "../rendering/VulkanRenderer.h"
#include "../io/IOHandler.h"
#include "MeshDataManager.h"

class SceneManager {
private:
    // Camera(s)
    Camera camera;
    Camera shadowCamera;
    float camTranslateSensitivity, camRotateSensitivity;

    // Meshes
    std::shared_ptr<MeshDataManager> meshDataManager;

    // Textures
    std::vector<Texture> textures;

    // Shaders
    std::vector<VulkanShadowPassShader> shadowPassShaders;
    std::vector<VulkanDeferredPassGeometryShader> deferredPassGeometryShaders; // TODO: this probably doesn't need to be a vector
    std::vector<VulkanDeferredPassLightingShader> deferredPassLightingShaders;
    std::vector<VulkanPostProcessShader> postProcessingShaders;

    // Scene IDs
    uint32_t currentScene;

public:
    SceneManager() : camTranslateSensitivity(0.25f), camRotateSensitivity(0.05f), currentScene(0) {}
    ~SceneManager() {}

    // Creation
    void Initialize(const std::shared_ptr<MeshDataManager>& p);
    void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass_firstPostProcess, VkImageView firstPostProcessImageView, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass, std::vector<PostProcessingPass> postProcessingPasses, uint32_t sceneId);
    void CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass, std::vector<PostProcessingPass> postProcessingPasses);
    void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass_deferredLighting, VkImageView deferredLightingImageView, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass, std::vector<PostProcessingPass> postProcessingPasses);

    // IO
    void RegisterCallbacks(IOHandler* ioHandler);
    
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
