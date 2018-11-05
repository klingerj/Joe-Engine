#include "VulkanDepthBuffer.h"

void VulkanDepthBuffer::Cleanup(VkDevice device) {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
}

void VulkanDepthBuffer::Create(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const VulkanSwapChain& vulkanSwapChain) {
    VkFormat depthFormat = FindDepthFormat(physicalDevice);
    VkExtent2D extent = vulkanSwapChain.GetExtent();
    CreateImage(physicalDevice, device, extent.width, extent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);
    depthImageView = CreateImageView(device, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    TransitionImageLayout(device, commandPool, graphicsQueue, depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}
