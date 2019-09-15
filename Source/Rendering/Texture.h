#pragma once

#include "vulkan/vulkan.h"
#include "../Utils/Common.h"

#include "VulkanQueue.h"

namespace JoeEngine {
    class JETexture {
    private:
        VkImage m_textureImage;
        VkDeviceMemory m_textureImageMemory;
        VkImageView m_textureImageView;
        VkSampler m_textureSampler;

    public:
        JETexture(VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanQueue& graphicsQueue,
            VkCommandPool commandPool, const std::string& filepath) {
            CreateTextureImage(physicalDevice, device, commandPool, graphicsQueue, filepath);
            CreateTextureImageView(device);
            CreateTextureSampler(device);
        }
        ~JETexture() {}

        void Cleanup(VkDevice device);

        // Creation
        void CreateTextureImage(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::string& filepath);
        void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        void CreateTextureImageView(VkDevice device);
        void CreateTextureSampler(VkDevice device);

        // Getters
        VkImageView GetImageView() const {
            return m_textureImageView;
        }
        VkSampler GetSampler() const {
            return m_textureSampler;
        }
    };
}
