#pragma once

#include "vulkan/vulkan.h"

#include "../Utils/Common.h"
#include "VulkanQueue.h"

namespace JoeEngine {
    class JETextureLibrary {
    private:

        std::vector<VkImage> m_images;
        
        
        std::vector<VkDeviceMemory> m_deviceMemory;
        

        std::vector<VkImageView> m_imageViews;
        
        
        std::vector<VkSampler> m_samplers;

        uint32_t m_numTextures;


        void CreateTextureImage(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::string& filepath);
        
        
        void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        
        
        void CreateTextureImageView(VkDevice device);
        
        
        void CreateTextureSampler(VkDevice device);

    public:

        JETextureLibrary() = default;


        ~JETextureLibrary() = default;


        uint32_t CreateTexture(VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanQueue& graphicsQueue,
            VkCommandPool commandPool, const std::string& filepath) {
            CreateTextureImage(physicalDevice, device, commandPool, graphicsQueue, filepath);
            CreateTextureImageView(device);
            CreateTextureSampler(device);
            return m_numTextures++;
        }


        void Cleanup(VkDevice device);


        VkImageView GetImageViewAt(int i) const {
            return m_imageViews[i];
        }


        VkSampler GetSamplerAt(int i) const {
            return m_samplers[i];
        }
    };
}
