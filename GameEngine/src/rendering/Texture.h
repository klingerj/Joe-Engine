#pragma once

#include "vulkan/vulkan.h"

#include "VulkanQueue.h"

class Texture {
private:
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
public:
    Texture() {}
    ~Texture() {}

    void Cleanup(VkDevice device);

    // TODO: Move these to a header common with Mesh
    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, const VulkanQueue& graphicsQueue, VkCommandPool commandPool);
    uint32_t FindMemoryType(const VkPhysicalDevice& physDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // Creation
    void CreateBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateTextureImage(const VkDevice& device, const VkPhysicalDevice& physDevice, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, const std::string& filepath);
    void CreateImage(const VkDevice& device, const VkPhysicalDevice& physDevice, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void TransitionImageLayout(VkDevice device, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkDevice device, const VulkanQueue& graphicsQueue, VkCommandPool commandPool, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};
