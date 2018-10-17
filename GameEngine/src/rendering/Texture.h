#pragma once

#include "vulkan/vulkan.h"
#include "../utils/Common.h"

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
        CreateTextureImage(physicalDevice, device, commandPool, graphicsQueue, filepath);
        CreateTextureImageView(device);
        CreateTextureSampler(device);
    }
    ~Texture() {}

    void Cleanup(VkDevice device);

    // Creation
    void CreateTextureImage(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::string& filepath);
    void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
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
