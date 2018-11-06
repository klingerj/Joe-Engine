#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/Mesh.h"
#include "../rendering/VulkanShader.h"
#include "../rendering/VulkanRenderer.h"

class SceneManager {
private:
    // Camera(s)
    Camera camera;
    Camera shadowCamera;

    // Meshes
    std::vector<Mesh> meshes;

    // Textures
    std::vector<Texture> textures;

    // Shaders
    std::vector<VulkanMeshShader> meshShaders;
    std::vector<VulkanShadowPassShader> shadowPassShaders;
    std::vector<VulkanDeferredPassGeometryShader> deferredPassGeometryShaders;

public:
    SceneManager() {}
    ~SceneManager() {}

    void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);
    void CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);
    void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);
    void CleanupMeshesAndTextures(VkDevice device);
    void CleanupShaders(VkDevice device);
    void UpdateModelMatrices();
    void UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex);
    void BindResources(VkCommandBuffer commandBuffer, size_t index);
    void BindShadowPassResources(VkCommandBuffer commandBuffer);
    void BindDeferredPassResources(VkCommandBuffer commandBuffer);
};
