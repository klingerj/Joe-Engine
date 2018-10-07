#pragma once

#include "vulkan/vulkan.h"
#include "../Common.h"

#include "VulkanQueue.h"

class Texture {
private:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;
public:
    Texture(VkDevice device, VkPhysicalDevice physicalDevice, const VulkanQueue& graphicsQueue,
            VkCommandPool commandPool, const std::string& filepath) {
        CreateTextureImage(device, physicalDevice, graphicsQueue, commandPool, filepath);
        CreateTextureImageView(device);
        CreateTextureSampler(device);
    }
    ~Texture() {}

    void Cleanup(VkDevice device);

    // Creation
    void CreateImage(const VkDevice& device, const VkPhysicalDevice& physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void CreateTextureImage(const VkDevice& device, const VkPhysicalDevice& physDevice, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, const std::string& filepath);
    void TransitionImageLayout(VkDevice device, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkDevice device, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateTextureImageView(VkDevice device);
    void CreateTextureSampler(VkDevice device);

    // Getters
    VkImageView GetImageView() const {
        return textureImageView;
    }
    VkSampler GetSampler() const {
        return textureSampler;
    }
};
