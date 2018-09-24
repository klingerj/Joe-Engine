#pragma once

#include <fstream>
#include <vector>

#include "vulkan\vulkan.h"
#include "VulkanSwapChain.h"

class VulkanShader {
private:
    VkDevice const * device; // pointer to the Vulkan device in use
protected:
    VkPipelineLayout pipelineLayout;
public:
    VulkanShader() {}
    VulkanShader(const VkDevice* device, const VulkanSwapChain& swapChain,
                 const std::string& vertShader, const std::string& fragShader) : device(device) {
        // Read from the .spv files
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);

        // Create shader modules
        VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
        VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

        // Create the pipeline
        CreateGraphicsPipeline(vertShaderModule, fragShaderModule, swapChain);
    }

    ~VulkanShader() {
        vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
    }
    
    // Creation functions
    void CreateGraphicsPipeline(const VkShaderModule& vertShaderModule, const VkShaderModule& fragShaderModule, const VulkanSwapChain& swapChain);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    std::vector<char> VulkanShader::ReadFile(const std::string& filename) const;
};
