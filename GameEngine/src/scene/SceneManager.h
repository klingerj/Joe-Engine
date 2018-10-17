#pragma once

#include <vector>

#include "Camera.h"
#include "../rendering/Texture.h"
#include "../rendering/Mesh.h"
#include "../rendering/VulkanShader.h"

class SceneManager {
private:
    // Camera
    Camera camera;

    // Meshes
    std::vector<Mesh> meshes;

    // Textures
    std::vector<Texture> textures;

    // Shaders
    std::vector<VulkanShader> shaders;

public:
    SceneManager() {}
    ~SceneManager() {}

    void LoadScene(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, VkRenderPass renderPass, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain);
    void RecreateResources(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& vulkanSwapChain, VkRenderPass renderPass);
    void CleanupMeshesAndTextures(VkDevice device);
    void CleanupShaders(VkDevice device);
    void UpdateModelMatrices();
    void UpdateShaderUniformBuffers(VkDevice device, uint32_t imageIndex);
    void BindResources(VkCommandBuffer commandBuffer, size_t index);
};
