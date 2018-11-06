#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"
#include "Mesh.h" // TODO: make separate shader classes, this class shouldn't be limited to MeshVertex's binding/attribute description
#include "Texture.h"
#include "../scene/Camera.h"

struct UBO_ViewProj {
    glm::mat4 viewProj;
};

struct UBODynamic_ModelMat {
    glm::mat4* model = nullptr;
};

std::vector<char> ReadFile(const std::string& filename);
VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

// Mesh Shader: Draws meshes with model/view/projection matrices and a texture.

class VulkanMeshShader {
private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    // Buffer info
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers_ViewProj;
    std::vector<VkDeviceMemory> uniformBuffersMemory_ViewProj;
    std::vector<VkBuffer> uniformBuffers_ViewProj_Shadow;
    std::vector<VkDeviceMemory> uniformBuffersMemory_ViewProj_Shadow;
    size_t uboDynamicAlignment;
    UBODynamic_ModelMat ubo_Dynamic_ModelMat;
    std::vector<VkBuffer> uniformBuffers_Dynamic_Model;
    std::vector<VkDeviceMemory> uniformBuffersMemory_Dynamic_Model;

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
        const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device, size_t numSwapChainImages);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass, size_t numSwapChainImages);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices);

public:
    VulkanMeshShader() : uboDynamicAlignment(0), ubo_Dynamic_ModelMat() {}
    VulkanMeshShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, const OffscreenShadowPass& shadowPass, VkRenderPass renderPass,
                     size_t numModelMatrices, const Texture& texture, const std::string& vertShader, const std::string& fragShader) {
        // Read in shader code
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

        size_t numSwapChainImages = swapChain.GetImageViews().size();
        CreateUniformBuffers(physicalDevice, device, numSwapChainImages, numModelMatrices);
        CreateDescriptorSetLayout(device);
        CreateDescriptorPool(device, numSwapChainImages);
        CreateDescriptorSets(device, texture, shadowPass, numSwapChainImages);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanMeshShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const Camera& shadowCamera, const std::vector<Mesh>& meshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};

// Shadow pass Shadow: Render to depth from the perspective of a light source

class VulkanShadowPassShader {
private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    // Buffer info
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer uniformBuffers_ViewProj;
    VkDeviceMemory uniformBuffersMemory_ViewProj;
    size_t uboDynamicAlignment;
    UBODynamic_ModelMat ubo_Dynamic_ModelMat;
    VkBuffer uniformBuffers_Dynamic_Model;
    VkDeviceMemory uniformBuffersMemory_Dynamic_Model;

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule, VkExtent2D extent, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numModelMatrices);

public:
    VulkanShadowPassShader() : uboDynamicAlignment(0), ubo_Dynamic_ModelMat() {}
    VulkanShadowPassShader(VkPhysicalDevice physicalDevice, VkDevice device, VkRenderPass renderPass, VkExtent2D extent, size_t numModelMatrices,
                           const std::string& vertShader, const std::string& fragShader) {
        // Read in shader code
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

        CreateUniformBuffers(physicalDevice, device, numModelMatrices);
        CreateDescriptorSetLayout(device);
        CreateDescriptorPool(device);
        CreateDescriptorSets(device);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, extent, renderPass);
    }

    ~VulkanShadowPassShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, const Camera& camera, const std::vector<Mesh>& meshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};

// Mesh Shader: Draws meshes with model/view/projection matrices and a texture.

class VulkanDeferredPassGeometryShader {
private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorSet descriptorSet;

    // Buffer info
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer uniformBuffers_ViewProj;
    VkDeviceMemory uniformBuffersMemory_ViewProj;
    VkBuffer uniformBuffers_ViewProj_Shadow;
    VkDeviceMemory uniformBuffersMemory_ViewProj_Shadow;
    size_t uboDynamicAlignment;
    UBODynamic_ModelMat ubo_Dynamic_ModelMat;
    VkBuffer uniformBuffers_Dynamic_Model;
    VkDeviceMemory uniformBuffersMemory_Dynamic_Model;

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
        const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numModelMatrices);

public:
    VulkanDeferredPassGeometryShader() : uboDynamicAlignment(0), ubo_Dynamic_ModelMat() {}
    VulkanDeferredPassGeometryShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, const OffscreenShadowPass& shadowPass, VkRenderPass renderPass,
        size_t numModelMatrices, const Texture& texture, const std::string& vertShader, const std::string& fragShader) {
        // Read in shader code
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

        size_t numSwapChainImages = swapChain.GetImageViews().size();
        CreateUniformBuffers(physicalDevice, device, numModelMatrices);
        CreateDescriptorSetLayout(device);
        CreateDescriptorPool(device);
        CreateDescriptorSets(device, texture, shadowPass);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanDeferredPassGeometryShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, const Camera& camera, const Camera& shadowCamera, const std::vector<Mesh>& meshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};
