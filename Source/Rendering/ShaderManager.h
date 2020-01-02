#pragma once

#include <vector>
#include <exception>

#include "VulkanShader.h"
#include "VulkanDescriptor.h"
#include "VulkanSwapChain.h"

namespace JoeEngine {
    //! The JEShaderManger class.
    /*!
      Class that manages all shader resources, including shader pipeline objects and descriptor objects.
      Provides creation and getter convenience functions.
      \sa JEShader, JEVulkanDescriptor, \sa JEVulkanRenderer
    */
    class JEShaderManager {
    private:
        //! List of shader objects.
        std::vector<JEShader*> m_shaders;
        // going to have to use operator new for allocating these derived classes but that'll just have
        // to suck for now i guess, until I can implement some kind of custom allocator.

        //! List of descriptor objects.
        std::vector<JEVulkanDescriptor> m_descriptors;

        //! Number of stored shaders.
        uint32_t m_numShaders;

        //! Number of stored descriptors.
        uint32_t m_numDescriptors;

        //! Reference to the renderer's Vulkan logical device.
        VkDevice m_device;

    public:
        //! Default constructor.
        JEShaderManager() : JEShaderManager(VK_NULL_HANDLE) {}

        //! Primary constructor.
        JEShaderManager(VkDevice device) : m_numShaders(0), m_numDescriptors(0), m_device(device) {}

        //! Destructor (default).
        ~JEShaderManager() = default;

        //! Create new shader.
        /*!
          Creates a new shader given the necessary shader pipeline settings.
          \param device the Vulkan logical device.
          \param physicalDevice the Vulkan physical device.
          \param swapChain the Vulkan swap chain.
          \param materialComponent the material component with shader settings.
          \param numSourceTextures the number of source textures for the shader.
          \param renderPass the Vulkan render pass object during which the new shader will be used.
          \param vertPath the vertex shader file source path.
          \param fragPath the fragment shader file source path.
          \param type the shader pipeline type enum.
          \return a shader ID corresponding to the newly created shader object.
        */
        uint32_t CreateShader(VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanSwapChain& swapChain,
            const MaterialComponent& materialComponent, uint32_t numSourceTextures, VkRenderPass renderPass,
            const std::string& vertPath, const std::string& fragPath, PipelineType type) {

            JEShader* newShader = nullptr;
            uint32_t numUniformBuffers = 0;
            // TODO: make these hard-coded constants less bad
            // TODO: custom allocator?
            switch (type) {
            case FORWARD:
                numUniformBuffers = 0;
                if (materialComponent.m_materialSettings & RECEIVES_SHADOWS) {
                    numUniformBuffers = 1;
                }
                newShader = new JEForwardShader(materialComponent, numSourceTextures, numUniformBuffers, device, physicalDevice, swapChain, renderPass, vertPath, fragPath);
                break;
            case DEFERRED:
                numUniformBuffers = 0;
                if (materialComponent.m_materialSettings & RECEIVES_SHADOWS) {
                    numUniformBuffers = 1;
                }
                ++numUniformBuffers; // invView and invProj
                newShader = new JEDeferredShader(materialComponent, numSourceTextures, materialComponent.m_uniformData.size() + numUniformBuffers, device, physicalDevice,
                    swapChain, renderPass, vertPath, fragPath);
                break;
            case SHADOW:
                newShader = new JEShadowShader(materialComponent, 0, device, physicalDevice, { JE_DEFAULT_SHADOW_MAP_WIDTH, JE_DEFAULT_SHADOW_MAP_HEIGHT },
                    renderPass, vertPath, fragPath);
                break;
            case DEFERRED_GEOM:
                newShader = new JEDeferredGeometryShader(materialComponent, numSourceTextures, materialComponent.m_uniformData.size(), device, physicalDevice,
                    swapChain, renderPass, vertPath, fragPath);
                break;
            case TRANSLUCENT_OIT:
                /*newShader = new JEForwardTranslucentShader(materialComponent, numSourceTextures, materialComponent.m_uniformData.size(), device, physicalDevice,
                    swapChain, renderPass, true, vertPath, fragPath);*/
                break;
            case TRANSLUCENT_OIT_SORT:
                newShader = new JEOITSortShader(materialComponent, device, physicalDevice, swapChain, renderPass, vertPath, fragPath);
                break;
            case FORWARD_POINTS:
                newShader = new JEPointsShader(materialComponent, 1, 0, device, physicalDevice, swapChain, renderPass, vertPath, fragPath);
                break;
            default:
                throw std::runtime_error("Invalid shader pipeline type!");
            }
            
            m_shaders.push_back(newShader);
            return m_numShaders++;
        }

        //! Create descriptor.
        /*!
          Creates a new descriptor object given various images and buffers.
          \param device the Vulkan logical device.
          \param physicalDevice the Vulkan physical device.
          \param swapChain the Vulkan swap chain.
          \param imageViews list of Vulkan image view objects.
          \param samplers list of Vulkna texture sampler objects.
          \param bufferSizes list of sizes of each uniform buffer.
          \param ssboSizes list of sizes for each shader storage buffer.
          \param layout the Vulkan descriptor set layout for the new descriptor object.
          \param type the shader pipeline type that this descriptor will be used with (and must be compatible with).
          \param recreate flag indicating whether this function is being called to recreate an existing descriptor object
          or to create a new one.
          \param recreateIdx if we are recreating an existing descriptor object, this is the ID of that descriptor object.
          \return an ID corresponding to the newly created descriptor.
        */
        uint32_t CreateDescriptor(VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanSwapChain& swapChain,
            const std::vector<std::vector<VkImageView>>& imageViews, const std::vector<VkSampler>& samplers,
            const std::vector<uint32_t>& bufferSizes, const std::vector<uint32_t>& ssboSizes, VkDescriptorSetLayout layout,
            PipelineType type, bool recreate, uint32_t recreateIdx = UINT32_MAX) {
            if (recreate) {
                if (recreateIdx >= m_numDescriptors) {
                    throw std::runtime_error("Invalid descriptor ID");
                }

                m_descriptors[recreateIdx].Cleanup(device);
                m_descriptors[recreateIdx] = JEVulkanDescriptor(physicalDevice, device, swapChain.GetImageViews().size(),
                    imageViews, samplers, bufferSizes, ssboSizes, layout, type);
                return recreateIdx;
            } else {
                m_descriptors.emplace_back(JEVulkanDescriptor(physicalDevice, device, swapChain.GetImageViews().size(),
                    imageViews, samplers, bufferSizes, ssboSizes, layout, type));
                return m_numDescriptors++;
            }
        }
        
        //! Get shader object.
        /*!
          \param shaderID the ID of the shader object to access.
          \return the shader object corresponding to the given ID.
        */
        const JEShader* GetShaderAt(int shaderID) const {
            if (shaderID < 0 || shaderID >= m_numShaders) {
                throw std::runtime_error("Invalid shader ID");
            }

            return m_shaders[shaderID];
        }

        //! Get descriptor object.
        /*!
          \param descriptorID the ID of the descriptor object to access.
          \return the descriptor object corresponding to the given ID.
        */
        const JEVulkanDescriptor& GetDescriptorAt(int descriptorID) const {
            if (descriptorID < 0 || descriptorID >= m_numDescriptors) {
                throw std::runtime_error("Invalid descriptor ID");
            }

            return m_descriptors[descriptorID];
        }

        //! Update the uniform and shader storage buffers of the specified descriptor.
        /*!
          \param device the Vulkan logical device.
          \param descriptorID the ID of the descriptor object to update.
          \param imageIndex the currently active swap chain image index.
          \param buffers list of uniform data buffers
          \param bufferSizes list of uniform data buffer sizes
          \param ssboBuffers list of shader storage data buffers
          \param ssboSizes list of shader storage data buffer sizes
        */
        void UpdateBuffers(VkDevice device, uint32_t descriptorID, uint32_t imageIndex, const std::vector<const void*>& buffers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<const void*>& ssboBuffers, const std::vector<uint32_t>& ssboSizes) {
            if (descriptorID < 0 || descriptorID >= m_numDescriptors) {
                // TODO: return some default/debug material
                throw std::runtime_error("Invalid descriptor ID");
            }

            m_descriptors[descriptorID].UpdateDescriptorSets(device, imageIndex, buffers, bufferSizes, ssboBuffers, ssboSizes);
        }

        //! Cleanup all managed shader and descriptor objects.
        void Cleanup() {
            for (uint32_t i = 0; i < m_numShaders; ++i) {
                m_shaders[i]->Cleanup();
                delete m_shaders[i];
            }

            for (uint32_t i = 0; i < m_numDescriptors; ++i) {
                m_descriptors[i].Cleanup(m_device);
            }
        }
    };
}
