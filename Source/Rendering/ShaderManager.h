#pragma once

#include <vector>
#include <exception>

#include "VulkanShader.h"
#include "VulkanDescriptor.h"
#include "VulkanSwapChain.h"

namespace JoeEngine {
    class JEShaderManager {
    private:

        std::vector<JEShader*> m_shaders;
        // going to have to use operator new for allocating these derived classes but that'll just have
        // to suck for now i guess, until I can implement some kind of custom allocator.

        std::vector<JEVulkanDescriptor> m_descriptors;

        uint32_t m_numShaders;
        uint32_t m_numDescriptors;

        VkDevice m_device;

    public:
        JEShaderManager() : JEShaderManager(VK_NULL_HANDLE) {}
        JEShaderManager(VkDevice device) : m_numShaders(0), m_numDescriptors(0), m_device(device) {}
        ~JEShaderManager() = default;

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
                newShader = new JEForwardTranslucentShader(materialComponent, numSourceTextures, materialComponent.m_uniformData.size(), device, physicalDevice,
                    swapChain, renderPass, true, vertPath, fragPath);
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

        const JEShader* GetShaderAt(int shaderID) const {
            if (shaderID < 0 || shaderID >= m_numShaders) {
                throw std::runtime_error("Invalid shader ID");
            }

            return m_shaders[shaderID];
        }

        const JEVulkanDescriptor& GetDescriptorAt(int descriptorID) const {
            if (descriptorID < 0 || descriptorID >= m_numDescriptors) {
                throw std::runtime_error("Invalid descriptor ID");
            }

            return m_descriptors[descriptorID];
        }

        void UpdateBuffers(VkDevice device, uint32_t descriptorID, uint32_t imageIndex, const std::vector<const void*>& buffers, const std::vector<uint32_t>& bufferSizes,
            const std::vector<const void*>& ssboBuffers, const std::vector<uint32_t>& ssboSizes) {
            if (descriptorID < 0 || descriptorID >= m_numDescriptors) {
                // TODO: return some default/debug material
                throw std::runtime_error("Invalid descriptor ID");
            }

            m_descriptors[descriptorID].UpdateDescriptorSets(device, imageIndex, buffers, bufferSizes, ssboBuffers, ssboSizes);
        }

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
