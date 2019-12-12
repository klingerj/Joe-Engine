#include "VulkanDescriptor.h"

namespace JoeEngine {
    void JEVulkanDescriptor::Cleanup(VkDevice device) {
        if (m_descriptorPool != VK_NULL_HANDLE) {
            vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
            // Destroying pool destroys sets too
        }

        for (uint32_t i = 0; i < m_uniformBuffers.size(); ++i) {
            for (uint32_t j = 0; j < m_uniformBuffers[i].size(); ++j) {
                if (m_uniformBuffers[i][j] != VK_NULL_HANDLE) {
                    vkDestroyBuffer(device, m_uniformBuffers[i][j], nullptr);
                }
            }
        }

        for (uint32_t i = 0; i < m_uniformDeviceMemory.size(); ++i) {
            for (uint32_t j = 0; j < m_uniformDeviceMemory[i].size(); ++j) {
                if (m_uniformDeviceMemory[i][j] != VK_NULL_HANDLE) {
                    vkFreeMemory(device, m_uniformDeviceMemory[i][j], nullptr);
                }
            }
        }

        for (uint32_t i = 0; i < m_ssboBuffers.size(); ++i) {
            for (uint32_t j = 0; j < m_ssboBuffers[i].size(); ++j) {
                if (m_ssboBuffers[i][j] != VK_NULL_HANDLE) {
                    vkDestroyBuffer(device, m_ssboBuffers[i][j], nullptr);
                }
            }
        }

        for (uint32_t i = 0; i < m_ssboDeviceMemory.size(); ++i) {
            for (uint32_t j = 0; j < m_ssboDeviceMemory[i].size(); ++j) {
                if (m_ssboDeviceMemory[i][j] != VK_NULL_HANDLE) {
                    vkFreeMemory(device, m_ssboDeviceMemory[i][j], nullptr);
                }
            }
        }
    }

    void JEVulkanDescriptor::CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
        const std::vector<uint32_t>& bufferSizes) {
        m_uniformBuffers.resize(bufferSizes.size());
        m_uniformDeviceMemory.resize(bufferSizes.size());

        for (uint32_t i = 0; i < bufferSizes.size(); ++i) {
            const VkDeviceSize bufferSize = bufferSizes[i];
            m_uniformBuffers[i].resize(numSwapChainImages);
            m_uniformDeviceMemory[i].resize(numSwapChainImages);
            for (uint32_t j = 0; j < numSwapChainImages; ++j) {
                CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                             m_uniformBuffers[i][j], m_uniformDeviceMemory[i][j]);
            }
        }
    }
    
    void JEVulkanDescriptor::CreateSSBOBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
        const std::vector<uint32_t>& ssboSizes) {
        m_ssboBuffers.resize(ssboSizes.size());
        m_ssboDeviceMemory.resize(ssboSizes.size());

        for (uint32_t i = 0; i < ssboSizes.size(); ++i) {
            const VkDeviceSize bufferSize = ssboSizes[i];
            m_ssboBuffers[i].resize(numSwapChainImages);
            m_ssboDeviceMemory[i].resize(numSwapChainImages);
            for (uint32_t j = 0; j < numSwapChainImages; ++j) {
                CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                    m_ssboBuffers[i][j], m_ssboDeviceMemory[i][j]);
            }
        }
    }

    void JEVulkanDescriptor::CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages, uint32_t numSourceTextures,
        uint32_t numUniformBuffers, uint32_t numSSBOBuffers) {
        std::vector<VkDescriptorPoolSize> poolSizes;
        VkDescriptorPoolSize poolSize;

        // SSBOs
        poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSize.descriptorCount = static_cast<uint32_t>(numSwapChainImages);
        for (uint32_t i = 0; i < numSSBOBuffers; ++i) {
            poolSizes.push_back(poolSize);
        }

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

    void JEVulkanDescriptor::CreateDescriptorSets(VkDevice device, uint32_t numSwapChainImages,
        const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers, const std::vector<uint32_t>& bufferSizes,
        const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout descSetLayout, PipelineType type) {
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
            std::vector<VkDescriptorBufferInfo> storageBufferInfos;
            for (uint32_t j = 0; j < ssboSizes.size(); ++j) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_ssboBuffers[j][i];
                bufferInfo.offset = 0;
                bufferInfo.range = ssboSizes[j];
                storageBufferInfos.push_back(bufferInfo);
            }

            std::vector<VkDescriptorBufferInfo> uniformBufferInfos;
            for (uint32_t j = 0; j < bufferSizes.size(); ++j) {
                VkDescriptorBufferInfo bufferInfo = {};
                bufferInfo.buffer = m_uniformBuffers[j][i];
                bufferInfo.offset = 0;
                bufferInfo.range = bufferSizes[j];
                uniformBufferInfos.push_back(bufferInfo);
            }

            std::vector<VkDescriptorImageInfo> imageInfos;
            if (imageViews.size() > 0) {
                for (uint32_t j = 0; j < imageViews[0].size(); ++j) {
                    VkDescriptorImageInfo imageInfo = {};
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    if ((type == DEFERRED && (j == 2 || j == 3)) ||
                        (type == FORWARD && (j == 4))) { // Depth stencil G-buffer, shadow map
                        imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
                    }
                    imageInfo.imageView = imageViews[i][j];
                    imageInfo.sampler = samplers[j];
                    imageInfos.push_back(imageInfo);
                }
            }

            std::vector<VkWriteDescriptorSet> descriptorWrites;
            for (uint32_t j = 0; j < storageBufferInfos.size(); ++j) {
                VkWriteDescriptorSet descWrite = {};
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.dstSet = m_descriptorSets[i];
                descWrite.dstBinding = j;
                descWrite.dstArrayElement = 0;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                descWrite.descriptorCount = 1;
                descWrite.pBufferInfo = &storageBufferInfos[j];
                descWrite.pImageInfo = nullptr;
                descWrite.pNext = nullptr;
                descriptorWrites.push_back(descWrite);
            }

            for (uint32_t j = 0; j < uniformBufferInfos.size(); ++j) {
                VkWriteDescriptorSet descWrite = {};
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.dstSet = m_descriptorSets[i];
                descWrite.dstBinding = j + storageBufferInfos.size();
                descWrite.dstArrayElement = 0;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                descWrite.descriptorCount = 1;
                descWrite.pBufferInfo = &uniformBufferInfos[j];
                descWrite.pImageInfo = nullptr;
                descWrite.pNext = nullptr;
                descriptorWrites.push_back(descWrite);
            }

            for (uint32_t j = 0; j < imageInfos.size(); ++j) {
                VkWriteDescriptorSet descWrite = {};
                descWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descWrite.dstSet = m_descriptorSets[i];
                descWrite.dstBinding = j + storageBufferInfos.size() + uniformBufferInfos.size();
                descWrite.dstArrayElement = 0;
                descWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                descWrite.descriptorCount = 1;
                descWrite.pImageInfo = &imageInfos[j];
                descWrite.pBufferInfo = nullptr;
                descWrite.pNext = nullptr;
                descriptorWrites.push_back(descWrite);
            }

            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }
    }

    void JEVulkanDescriptor::UpdateDescriptorSets(VkDevice device, uint32_t imageIndex, const std::vector<const void*>& buffers, const std::vector<uint32_t>& bufferSizes,
        const std::vector<const void*>& ssboBuffers, const std::vector<uint32_t>& ssboSizes) {

        // Uniform buffers
        for (uint32_t i = 0; i < m_uniformBuffers.size(); ++i) {
            void* data;
            vkMapMemory(device, m_uniformDeviceMemory[i][imageIndex], 0, bufferSizes[i], 0, &data);
            if (buffers[i] == nullptr) {
                memset(data, 0, bufferSizes[i]);
            } else {
                memcpy(data, buffers[i], bufferSizes[i]);
            }
            vkUnmapMemory(device, m_uniformDeviceMemory[i][imageIndex]);
        }

        // SSBOs
        for (uint32_t i = 0; i < m_ssboBuffers.size(); ++i) {
            void* data;
            if (ssboSizes[i] > 0) {
                vkMapMemory(device, m_ssboDeviceMemory[i][imageIndex], 0, ssboSizes[i], 0, &data);
                if (ssboBuffers[i] == nullptr) {
                    // TODO: create some debug value parameter for this, it is highly usage specific
                    memset(data, UINT32_MAX, ssboSizes[i]);
                } else {
                    memcpy(data, ssboBuffers[i], ssboSizes[i]);
                }
                vkUnmapMemory(device, m_ssboDeviceMemory[i][imageIndex]);
            }
        }
    }
}
