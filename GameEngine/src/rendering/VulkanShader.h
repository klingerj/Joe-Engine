#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"

#include "Mesh.h" // TODO: make separate shader classes, this class shouldn't be limited to MeshVertex's binding/attribute description
#include "Camera.h"

struct UBO_MVP {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
public:
    VulkanShader() {}
    VulkanShader(const VkDevice& device, const VkPhysicalDevice physDevice, const VulkanSwapChain& swapChain, const VkRenderPass& renderPass,
                 const std::string& vertShader, const std::string& fragShader) {
        // Read in shader code
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

        size_t numSwapChainImages = swapChain.GetImageViews().size();
        CreateUniformBuffer(device, physDevice, numSwapChainImages);
        CreateDescriptorSetLayout(device);
        CreateDescriptorPool(device, numSwapChainImages);
        CreateDescriptorSets(device, numSwapChainImages);

        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanShader() {}

    void Cleanup(const VkDevice& device);
    
    // Creation functions
    void CreateGraphicsPipeline(const VkDevice& device, const VkShaderModule& vertShaderModule, const VkShaderModule& fragShaderModule,
                                const VulkanSwapChain& swapChain, const VkRenderPass& renderPass);
    VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code);
    std::vector<char> VulkanShader::ReadFile(const std::string& filename) const;

    // Descriptors
    void CreateDescriptorPool(VkDevice device, size_t numSwapChainImages);
    void CreateDescriptorSetLayout(VkDevice device);
    void CreateDescriptorSets(VkDevice device, size_t numSwapChainImages);
    void CreateUniformBuffer(VkDevice device, VkPhysicalDevice physDevice, size_t numSwapChainImages);
    void UpdateUniformBuffer(VkDevice device, uint32_t currentImage, const Camera& camera);
    void BindDescriptorSets(const VkCommandBuffer& commandBuffer, size_t descriptorSetIndex);

    // Getters
    const VkPipeline& GetPipeline() {
        return graphicsPipeline;
    }
};
