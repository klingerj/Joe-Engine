#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"
#include "VulkanRenderer.h"
#include "Texture.h"
#include "../scene/Camera.h"

struct UBO_ViewProj {
    glm::mat4 viewProj;
};

struct UBO_ViewProj_Inv {
    glm::mat4 invProj;
    glm::mat4 invView;
};

struct UBODynamic_ModelMat {
    glm::mat4* model = nullptr;
};

std::vector<char> ReadFile(const std::string& filename);
VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

// Post Processing Shader: Draws a meshes with a texture uniform.

class VulkanPostProcessShader {
private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    VkDescriptorPool descriptorPool;
    VkDescriptorSetLayout descriptorSetLayout;
    std::vector<VkDescriptorSet> descriptorSets;

    // Buffer info
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
        const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device, size_t numSwapChainImages);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const PostProcessingPass& postProcessingPass, VkImageView postImageView, size_t numSwapChainImages);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices);

public:
    VulkanPostProcessShader() {}
    VulkanPostProcessShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, const PostProcessingPass& postProcessingPass, VkRenderPass renderPass,
                     size_t numModelMatrices, VkImageView postImageView, const std::string& vertShader, const std::string& fragShader) {
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
        CreateDescriptorSets(device, postProcessingPass, postImageView, numSwapChainImages);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanPostProcessShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const Camera& shadowCamera, const glm::mat4* modelMatrices, uint32_t numMeshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
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

    void UpdateUniformBuffers(VkDevice device, const Camera& camera, const glm::mat4* modelMatrices, uint32_t numMeshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};

// Deferred Geometry Pass Shader: Renders meshes to g-buffers

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
    size_t uboDynamicAlignment;
    UBODynamic_ModelMat ubo_Dynamic_ModelMat;
    VkBuffer uniformBuffers_Dynamic_Model;
    VkDeviceMemory uniformBuffersMemory_Dynamic_Model;

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
        const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const Texture& texture);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numModelMatrices);

public:
    VulkanDeferredPassGeometryShader() : uboDynamicAlignment(0), ubo_Dynamic_ModelMat() {}
    VulkanDeferredPassGeometryShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, VkRenderPass renderPass,
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
        CreateDescriptorSets(device, texture);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanDeferredPassGeometryShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, const Camera& camera, const glm::mat4* modelMatrices, uint32_t numMeshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};

// Deferred Lighting Pass: Renders a scene using G-buffers

class VulkanDeferredPassLightingShader {
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

    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
        const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    void CreateDescriptorPool(VkDevice device, size_t numSwapChainImages);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const Texture& texture, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass, size_t numSwapChainImages);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices);

public:
    VulkanDeferredPassLightingShader() {}
    VulkanDeferredPassLightingShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, const OffscreenShadowPass& shadowPass, const OffscreenDeferredPass& deferredPass,
        VkRenderPass renderPass, size_t numModelMatrices, const Texture& texture, const std::string& vertShader, const std::string& fragShader) {
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
        CreateDescriptorSets(device, texture, shadowPass, deferredPass, numSwapChainImages);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanDeferredPassLightingShader() {}

    void Cleanup(VkDevice device);

    void UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const Camera& shadowCamera);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex);

    // Getters
    VkPipeline GetPipeline() const {
        return graphicsPipeline;
    }
};
