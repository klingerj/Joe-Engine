#include "VulkanDescriptor.h"

namespace JoeEngine {
    void JEVulkanDescriptor::Cleanup() {
        if (m_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(m_device, m_descriptorPool, nullptr);
        }

        for (uint32_t i = 0; i < m_buffers.size(); ++i) {
            for (uint32_t j = 0; j < m_buffers[i].size(); ++j) {
                if (m_buffers[i][j] != VK_NULL_HANDLE) {
                    vkDestroyBuffer(m_device, m_buffers[i][j], nullptr);
                }
            }
        }

        for (uint32_t i = 0; i < m_deviceMemory.size(); ++i) {
            for (uint32_t j = 0; j < m_deviceMemory[i].size(); ++j) {
                if (m_deviceMemory[i][j] != VK_NULL_HANDLE) {
                    vkFreeMemory(m_device, m_deviceMemory[i][j], nullptr);
                }
            }
        }
    }

    void JEVulkanDescriptor::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
        const std::vector<void*>& buffers, const std::vector<uint32_t>& bufferSizes) {
        m_buffers.resize(buffers.size());
        m_deviceMemory.resize(buffers.size());

        for (uint32_t i = 0; i < buffers.size(); ++i) {
            const VkDeviceSize bufferSize = bufferSizes[i];
            m_buffers[i].resize(numSwapChainImages);
            m_deviceMemory[i].resize(numSwapChainImages);
            for (uint32_t j = 0; j < numSwapChainImages; ++j) {
                CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             m_buffers[i][j], m_deviceMemory[i][j]);
            }
        }
    }

    void JEVulkanDescriptor::CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages, uint32_t numSourceTextures, uint32_t numUniformBuffers) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        VkDescriptorPoolSize poolSize;

        // Uniform buffers
        poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(numSwapChainImages);
        for (uint32_t i = 0; i < numUniformBuffers; ++i) {
            poolSizes.push_back(poolSize);
        }

        // Uniform textures
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount = static_cast<uint32_t>(numSwapChainImages);
        for (uint32_t i = 0; i < numSourceTextures; ++i) {
            poolSizes.push_back(poolSize);
        }

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = static_cast<uint32_t>(numSwapChainImages);

        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create descriptor pool!");
        }
    }

    void JEVulkanDescriptor::CreateDescriptorSets(VkDevice device, const MaterialComponent& materialComponent, uint32_t numSwapChainImages,
        const std::vector<VkImageView>& imageViews, const std::vector<VkSampler>& samplers, const std::vector<uint32_t>& bufferSizes,
        VkDescriptorSetLayout descSetLayout) {
        // TODO: we will have multiple shadow pass and g-buffer pass image views eventually
        // Maybe we call this multiple times, one per swap chain image index. Need to move the allocation code, maybe to descriptor pool creation function above?
        std::vector<VkDescriptorSetLayout> layouts(numSwapChainImages, descSetLayout);

        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(numSwapChainImages);
        allocInfo.pSetLayouts = layouts.data();

        m_descriptorSets.resize(numSwapChainImages);
        if (vkAllocateDescriptorSets(device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate descriptor sets!");
        }

        for (uint32_t i = 0; i < numSwapChainImages; ++i) {
            std::vector<VkDescriptorBufferInfo> bufferInfos;
            for (uint32_t j = 0; j < bufferSizes.size(); ++j) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_buffers[j][i];
                bufferInfo.offset = 0;
                bufferInfo.range = bufferSizes[j];
                bufferInfos.push_back(bufferInfo);
            }

            std::vector<VkDescriptorImageInfo> imageInfos;
            for (uint32_t j = 0; j < imageViews.size(); ++j) {
                VkDescriptorImageInfo imageInfo = {};
                imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                imageInfo.imageView = imageViews[j];
                imageInfo.sampler = samplers[j];
                imageInfos.push_back(imageInfo);
            }

            std::vector<VkWriteDescriptorSet> descriptorWrites;
            for (uint32_t j = 0; j < bufferInfos.size(); ++j) {
                VkWriteDescriptorSet descWrite;
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.dstSet = m_descriptorSets[i];
                descWrite.dstBinding = j;
                descWrite.dstArrayElement = 0;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descWrite.descriptorCount = 1;
                descWrite.pBufferInfo = &bufferInfos[j];
            }

            for (uint32_t j = 0; j < imageInfos.size(); ++j) {
                VkWriteDescriptorSet descWrite;
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.dstSet = m_descriptorSets[i];
                descWrite.dstBinding = j + bufferInfos.size();
                descWrite.dstArrayElement = 0;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descWrite.descriptorCount = 1;
                descWrite.pImageInfo = &imageInfos[j];
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void JEVulkanDescriptor::UpdateDescriptorSets(uint32_t imageIndex, const std::vector<void*>& buffers, const std::vector<uint32_t>& bufferSizes) {
        for (uint32_t i = 0; i < m_buffers.size(); ++i) {
            void* data;
            vkMapMemory(m_device, m_deviceMemory[i][imageIndex], 0, bufferSizes[i], 0, &data); // TODO: debug bad ptr error here
            memcpy(data, buffers[i], bufferSizes[i]);
            vkUnmapMemory(m_device, m_deviceMemory[i][imageIndex]);
        }
    }
}
