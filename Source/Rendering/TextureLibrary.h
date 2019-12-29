#pragma once

#include "vulkan/vulkan.h"

#include "../Utils/Common.h"
#include "VulkanQueue.h"

namespace JoeEngine {
    //!  The JETextureLibrary class.
    /*!
      Class for managing Vulkan texture data. Provides creation and getter convenience functions.
    */
    class JETextureLibrary {
    private:
        //! List of Vulkan images.
        std::vector<VkImage> m_images;
        
        //! List of Vulkan device memory buffers.
        std::vector<VkDeviceMemory> m_deviceMemory;
        
        //! List of Vulkan image views.
        std::vector<VkImageView> m_imageViews;
        
        //! List of samplers.
        std::vector<VkSampler> m_samplers;

        //! Number of textures currently being managed.
        uint32_t m_numTextures;

        //! Loads a texture from the specified filepath and copies it to a new Vulkan image object.
        /*!
          \param physicalDevice the Vulkan physical device.
          \param device the Vulkan logical device.
          \param commandPool the Vulkan command pool for issuing necessary texture creation commands.
          \param graphicsQueue the Vulkan graphics queue.
          \param filepath texture file source path.
        */
        void CreateTextureImage(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::string& filepath);
        
        //! Copies a Vulkan texture buffer to a Vulkan image.
        /*!
          \param device The Vulkan logical device.
          \param commandPool the Vulkan command pool.
          \param graphicsQueue the Vulkan graphics queue.
          \param buffer the Vulkan buffer containing the texture data.
          \param image the Vulkan image to copy the buffer contents to.
          \param width the texture width.
          \param height the texture height.
        */
        void CopyBufferToImage(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
        
        //! Creates a Vulkan texture image view from a Vulkan texture.
        /*!
          \param device the Vulkan logical device.
        */
        void CreateTextureImageView(VkDevice device);
        
        //! Creates a Vulkan sampler for the texture.
        /*!
          \param device the Vulkan logical device.
        */
        void CreateTextureSampler(VkDevice device);

    public:
        //! Default constructor.
        JETextureLibrary() : m_numTextures(0) {}

        //! Destructor (default).
        ~JETextureLibrary() = default;

        //! Create texture helper function.
        /*!
          Public user API function for loading and creating a new texture.
          \param device the Vulkan logical device.
          \param physicalDevice the Vulkan physical device.
          \param graphicsQueue the Vulkan graphics queue.
          \param commandPool the Vulkan command pool.
          \param filepath the texture file source path.
          \return a texture ID that corresponds to the new texture.
        */
        uint32_t CreateTexture(VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanQueue& graphicsQueue,
            VkCommandPool commandPool, const std::string& filepath) {
            m_images.push_back(VK_NULL_HANDLE);
            m_deviceMemory.push_back(VK_NULL_HANDLE);
            m_imageViews.push_back(VK_NULL_HANDLE);
            m_samplers.push_back(VK_NULL_HANDLE);
            CreateTextureImage(physicalDevice, device, commandPool, graphicsQueue, filepath);
            CreateTextureImageView(device);
            CreateTextureSampler(device);
            return m_numTextures++;
        }

        //! Cleanup Vulkan objects.
        /*!
          \param device the Vulkan logical device.
        */
        void Cleanup(VkDevice device);

        //! Get image view at index.
        /*!
          \param i the texture ID.
          \return the texture image view corresponding to the given ID.
        */
        VkImageView GetImageViewAt(int i) const {
            return m_imageViews[i];
        }

        //! Get texture sampler at index.
        /*!
          \param i the texture ID.
          \return the texture sampler corresponding to the given ID.
        */
        VkSampler GetSamplerAt(int i) const {
            return m_samplers[i];
        }
    };
}
