#pragma once

#include <vector>

#include "VulkanRenderingTypes.h"
#include "../Components/Material/MaterialComponent.h"

namespace JoeEngine {
    //! The JEVulkanDescriptor class.
    /*!
      Class that manages descriptor sets and device memory buffers for materials in the Joe Engine.
    */
    class JEVulkanDescriptor {
    private:
        //! Descriptor pool.
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;

        //! List of descriptor sets (one per swap chain image).
        std::vector<VkDescriptorSet> m_descriptorSets;

        // Device memory buffers:
        // First is the list per actual data buffer.
        // Second is per swapchain image.

        //! List of per-swap-chain-image uniform memory buffer.
        std::vector<std::vector<VkBuffer>> m_uniformBuffers;

        //! List of per-swap-chain-image uniform device memory buffers.
        std::vector<std::vector<VkDeviceMemory>> m_uniformDeviceMemory;
        
        //! List of per-swap-chain-image shader storage buffers.
        std::vector<std::vector<VkBuffer>> m_ssboBuffers;

        //! List of per-swap-chain-image shader storage device memory buffers.
        std::vector<std::vector<VkDeviceMemory>> m_ssboDeviceMemory;

        //! Create uniform buffers.
        /*!
          Creates the specified number of uniform buffers on the specified physical/logical Vulkan devices given the specified
          sizes.
          \param physicalDevice the Vulkan physical device.
          \param device the Vulkan logical device.
          \param numSwapChainImages the number of swap chain images, i.e. the number of copies of each uniform buffer.
          \param bufferSizes the size of each buffer.
        */
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages, 
                                  const std::vector<uint32_t>& bufferSizes);

        //! Create shader storage buffers.
        /*!
          Creates the specified number of shader storage buffers on the specified physical/logical Vulkan devices given the specified
          sizes.
          \param physicalDevice the Vulkan physical device.
          \param device the Vulkan logical device.
          \param numSwapChainImages the number of swap chain images, i.e. the number of copies of each shader storage buffer.
          \param ssboSizes the size of each buffer.
        */
        void CreateSSBOBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
            const std::vector<uint32_t>& ssboSizes);

        //! Create descriptor pool.
        /*!
          Creates a Vulkan decriptor pool given the specified quantity of each type of uniform or storage buffer.
          \param device the Vulkan logical device.
          \param numSwapChainImages the number of active swap chain images, i.e. the number of descriptor sets to allocate from the pool.
          \param numSourceTextures the number of uniform image samplers to allocate from the pool.
          \param numUniformBuffers the number of uniform buffers to allocate from the pool.
          \param numSSBOBuffers the number of shader storage buffers to allocate from the pool.
        */
        void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages, uint32_t numSourceTextures,
            uint32_t numUniformBuffers, uint32_t numSSBOBuffers);

        //! Create descriptor sets.
        /*!
          Create descriptor set objects from the descriptor pool with the specified buffer data.
          \param device the Vulkan logical device.
          \param numSwapChainImages number of active swap chain images, i.e. the number of descriptor sets to allocate from the pool.
          \param imageViews list of image views for the descriptor set.
          \param samplers list of samplers for each image view (one per image view in the list 'imageViews').
          \param bufferSizes size of each uniform buffer for the descriptor set.
          \param ssboSizes size of each shader storage buffer for the descriptor set.
          \param descSetLayout layout for the descriptor set. Needed for Vulkan calls.
          \param type the render pipeline type for the shader this descriptor is to be bound with.
        */
        void CreateDescriptorSets(VkDevice device, uint32_t numSwapChainImages,
            const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout descSetLayout, PipelineType type);

    public:
        //! Default constructor (deleted).
        JEVulkanDescriptor() = delete;

        //! Constructor.
        /*!
          Creates all necessary Vulkan descriptor objects (pool and sets) and allocates all necessary buffer objects.
        */
        JEVulkanDescriptor(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages,
            const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers,
            const std::vector<uint32_t>& bufferSizes, const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout descSetLayout, PipelineType type) {
            if (imageViews.size() == 0) {
                CreateDescriptorPool(device, numSwapChainImages, 0, bufferSizes.size(), ssboSizes.size());
            } else {
                CreateDescriptorPool(device, numSwapChainImages, imageViews[0].size(), bufferSizes.size(), ssboSizes.size());
            }
            
            CreateUniformBuffers(physicalDevice, device, numSwapChainImages, bufferSizes);
            CreateSSBOBuffers(physicalDevice, device, numSwapChainImages, ssboSizes);
            CreateDescriptorSets(device, numSwapChainImages, imageViews, samplers, bufferSizes, ssboSizes, descSetLayout, type);
        }

        //! Destructor (default).
        ~JEVulkanDescriptor() = default;

        //! Cleanup all Vulkan objects and memory.
        //! \param device the Vulkan logical device to clean up with.
        void Cleanup(VkDevice device);

        //! Bind descriptor sets.
        /*!
          Binds the specified descriptor set. Called during command buffer recording.
          \param commandBuffer the command buffer to record the bind command to.
          \param pipelineLayout the pipeline layout for the descriptor set.
          \param descrSetIndex the specific descriptor set index to bind.
          \param imageIndex the currently active swap chain image index.
        */
        void BindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, uint32_t descrSetIndex, uint32_t imageIndex) const {
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, descrSetIndex, 1, &m_descriptorSets[imageIndex], 0, nullptr);
        }

        //! Update descriptor sets.
        /*!
          Updates all descriptor set uniform and shader storage memory buffers to the data provided.
          \param device the Vulkan logical device.
          \param imageIndex the currently active swap chain image index.
          \param buffers list of new uniform buffer data to copy to the GPU.
          \param bufferSizes size of each element in the parameter 'buffers'.
          \param ssboBuffers list of new shader storage buffer data to copy to the GPU.
          \param ssboSizes size of each element in the parameter 'ssboBuffers'.
        */
        void UpdateDescriptorSets(VkDevice device, uint32_t imageIndex, const std::vector<const void*>& buffers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<const void*>& ssboBuffers, const std::vector<uint32_t>& ssboSizes);
    };
}
