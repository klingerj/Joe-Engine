#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"

#include "Mesh.h" // TODO: make separate shader classes, this class shouldn't be limited to MeshVertex's binding/attribute description
#include "Texture.h"
#include "../scene/Camera.h"

struct UBO_ViewProj {
    glm::mat4 viewProj;
};

struct UBODynamic_ModelMat {
    glm::mat4 *model = nullptr;
};

// Class to wrap pipelines and descriptors

class VulkanShader {
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
    size_t uboDynamicAlignment;
    UBODynamic_ModelMat ubo_Dynamic_ModelMat;
    std::vector<VkBuffer> uniformBuffers_Dynamic_Model;
    std::vector<VkDeviceMemory> uniformBuffersMemory_Dynamic_Model;

public:
    VulkanShader() : uboDynamicAlignment(0), ubo_Dynamic_ModelMat() {}
    VulkanShader(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanSwapChain& swapChain, VkRenderPass renderPass, size_t numModelMatrices,
                 const Texture& texture, const std::string& vertShader, const std::string& fragShader) {
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
        CreateDescriptorSets(device, texture, numSwapChainImages);
        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanShader() {}

    void Cleanup(VkDevice device);
    
    // Creation functions
    void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
                                const VulkanSwapChain& swapChain, VkRenderPass renderPass);
    VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);
    std::vector<char> VulkanShader::ReadFile(const std::string& filename) const;

    // Descriptors
    void CreateDescriptorPool(VkDevice device, size_t numSwapChainImages);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, const Texture& texture, size_t numSwapChainImages);
    void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, size_t numSwapChainImages, size_t numModelMatrices);
    void UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const Camera& camera, const std::vector<Mesh>& meshes);
    void BindDescriptorSets(VkCommandBuffer commandBuffer, size_t descriptorSetIndex, uint32_t dynamicOffset);

    // Getters
    VkPipeline GetPipeline() {
        return graphicsPipeline;
    }
    size_t GetDynamicAlignment() const {
        return uboDynamicAlignment;
    }
};
