#pragma once

#include <fstream>
#include <vector>

#include "vulkan\vulkan.h"
#include "VulkanSwapChain.h"

class VulkanShader {
private:
    VkDevice const * device; // pointer to the Vulkan device in use
protected:
    VkShaderModule vertShaderModule;
    VkShaderModule fragShaderModule;
    VkPipelineLayout pipelineLayout;
public:
    VulkanShader() {}
    VulkanShader(const VkDevice* device, const VulkanSwapChain& swapChain,
                 const std::string& vertShader, const std::string& fragShader) : device(device) {
        auto vertShaderCode = ReadFile(vertShader);
        auto fragShaderCode = ReadFile(fragShader);
        vertShaderModule = CreateShaderModule(vertShaderCode);
        fragShaderModule = CreateShaderModule(fragShaderCode);
        CreateGraphicsPipeline(swapChain);
    }

    ~VulkanShader() {
        vkDestroyPipelineLayout(*device, pipelineLayout, nullptr);
    }
    
    void CreateGraphicsPipeline(const VulkanSwapChain& swapChain);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);

    static std::vector<char> ReadFile(const std::string& filename);
};
