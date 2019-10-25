#pragma once

#include <vector>

#include "VulkanRenderingTypes.h"
#include "../Components/Material/MaterialComponent.h"

namespace JoeEngine {
    class JEVulkanDescriptor {
    private:
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;
        std::vector<std::vector<VkBuffer>> m_buffers;
        std::vector<std::vector<VkDeviceMemory>> m_deviceMemory;
        
        const VkDevice m_device;

        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages, 
                                  const std::vector<uint32_t>& bufferSizes);
        void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages, uint32_t numSourceTextures, uint32_t numUniformBuffers);
        void CreateDescriptorSets(VkDevice device, uint32_t numSwapChainImages,
            const std::vector<VkImageView>& imageViews, const std::vector<VkSampler>& samplers, const std::vector<uint32_t>& bufferSizes,
            VkDescriptorSetLayout descSetLayout, PipelineType type);

    public:
        JEVulkanDescriptor() = delete;
        JEVulkanDescriptor(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
            const std::vector<VkImageView>& imageViews, const std::vector<VkSampler>& samplers,
            const std::vector<uint32_t>& bufferSizes, VkDescriptorSetLayout descSetLayout, PipelineType type) :
            m_device(device) {
            CreateDescriptorPool(device, numSwapChainImages, imageViews.size(), bufferSizes.size());
            CreateUniformBuffers(physicalDevice, device, numSwapChainImages, bufferSizes);
            CreateDescriptorSets(device, numSwapChainImages, imageViews, samplers, bufferSizes, descSetLayout, type);
        }
        ~JEVulkanDescriptor() {}

        void Cleanup();

        void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t imageIndex) const {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &m_descriptorSets[imageIndex], 0, nullptr);
        }

        void UpdateDescriptorSets(uint32_t imageIndex, const std::vector<void*>& buffers, const std::vector<uint32_t>& bufferSizes);
    };
}
