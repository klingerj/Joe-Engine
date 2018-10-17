#pragma once

#include <vector>

#include "vulkan/vulkan.h"
#include "VulkanSwapChain.h"
#include "../utils/Common.h"

class VulkanDepthBuffer {
private:
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

public:
    VulkanDepthBuffer() {}
    ~VulkanDepthBuffer() {}

    void Cleanup(VkDevice device);

    // Getters
    VkImageView GetImageView() const {
        return depthImageView;
    }

    void Create(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain);
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);
};
