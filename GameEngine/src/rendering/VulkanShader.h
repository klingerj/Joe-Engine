#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"

#include "Mesh.h" // TODO: make separate shader classes, this class shouldn't be limited to MeshVertex's binding/attribute description

class VulkanShader {
private:
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    
public:
    VulkanShader() {}
    VulkanShader(const VkDevice& device, const VulkanSwapChain& swapChain, const VkRenderPass& renderPass,
                 const std::string& vertShader, const std::string& fragShader) {
        // Read in shader code
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

        CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
    }

    ~VulkanShader() {}

    void Cleanup(const VkDevice& device);
    
    // Creation functions
    void CreateGraphicsPipeline(const VkDevice& device, const VkShaderModule& vertShaderModule, const VkShaderModule& fragShaderModule,
                                const VulkanSwapChain& swapChain, const VkRenderPass& renderPass);
    VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code);
    std::vector<char> VulkanShader::ReadFile(const std::string& filename) const;

    // Getters
    const VkPipeline& GetPipeline() {
        return graphicsPipeline;
    }
};
