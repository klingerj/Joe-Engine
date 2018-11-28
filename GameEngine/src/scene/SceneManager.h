#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/Mesh.h"
#include "../rendering/VulkanShader.h"
#include "../rendering/VulkanRenderer.h"
#include "../io/IOHandler.h"

class SceneManager {
private:
    // Camera(s)
    Camera camera;
    Camera shadowCamera;
    float camTranslateSensitivity, camRotateSensitivity;

    // Meshes
    std::vector<Mesh> meshes;
    static Mesh screenSpaceTriangle;

    // Textures
    std::vector<Texture> textures;

    // Shaders
    std::vector<VulkanMeshShader> meshShaders;
    std::vector<VulkanShadowPassShader> shadowPassShaders;
    std::vector<VulkanDeferredPassGeometryShader> deferredPassGeometryShaders;
    std::vector<VulkanDeferredPassLightingShader> deferredPassLightingShaders;

public:
    SceneManager() : camTranslateSensitivity(1.0f), camRotateSensitivity(0.1f) {}
    ~SceneManager() {}

    // Creation
    void Initialize();
    void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);
    void CreateShaders(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);
    void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass);

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
};
