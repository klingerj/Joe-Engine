#pragma once

#include <vector>

#include "VulkanRenderingTypes.h"
#include "../Components/Material/MaterialComponent.h"

namespace JoeEngine {
    class JEVulkanDescriptor {
    private:
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // First is the list per actual data buffer.
        // Second is per swapchain image.
        std::vector<std::vector<VkBuffer>> m_uniformBuffers;
        std::vector<std::vector<VkDeviceMemory>> m_uniformDeviceMemory;
        
        std::vector<std::vector<VkBuffer>> m_ssboBuffers;
        std::vector<std::vector<VkDeviceMemory>> m_ssboDeviceMemory;

        const VkDevice m_device;

        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages, 
                                  const std::vector<uint32_t>& bufferSizes);
        void CreateSSBOBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
            const std::vector<uint32_t>& ssboSizes);
        void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages, uint32_t numSourceTextures,
            uint32_t numUniformBuffers, uint32_t numSSBOBuffers);
        void CreateDescriptorSets(VkDevice device, uint32_t numSwapChainImages,
            const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout descSetLayout, PipelineType type);

    public:
        JEVulkanDescriptor() = delete;
        JEVulkanDescriptor(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
            const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers,
            const std::vector<uint32_t>& bufferSizes, const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout descSetLayout, PipelineType type) :
            m_device(device) {
            if (imageViews.size() == 0) {
                CreateDescriptorPool(device, numSwapChainImages, 0, bufferSizes.size(), ssboSizes.size());
            } else {
                CreateDescriptorPool(device, numSwapChainImages, imageViews[0].size(), bufferSizes.size(), ssboSizes.size());
            }
            
            CreateUniformBuffers(physicalDevice, device, numSwapChainImages, bufferSizes);
            CreateSSBOBuffers(physicalDevice, device, numSwapChainImages, ssboSizes);
            CreateDescriptorSets(device, numSwapChainImages, imageViews, samplers, bufferSizes, ssboSizes, descSetLayout, type);
        }
        ~JEVulkanDescriptor() {}

        void Cleanup();

        void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t descrSetIndex, uint32_t imageIndex) const {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descrSetIndex, 1, &m_descriptorSets[imageIndex], 0, nullptr);
        }

        void UpdateDescriptorSets(uint32_t imageIndex, const std::vector<const void*>& buffers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<const void*>& ssboBuffers, const std::vector<uint32_t>& ssboSizes);
    };
}
