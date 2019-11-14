#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include "VulkanSwapChain.h"
#include "VulkanRenderingTypes.h"
#include "Texture.h"
#include "TextureLibrary.h"
#include "../Scene/Camera.h"
#include "../Components/Material/MaterialComponent.h"

namespace JoeEngine {
    struct JE_PushConst_ViewProj {
        glm::mat4 viewProj;
    };

    struct JEUBO_ViewProj_Inv {
        glm::mat4 invProj;
        glm::mat4 invView;
    };

    struct JE_PushConst_ModelMat {
        glm::mat4 model;
    };

    std::vector<char> ReadFile(const std::string& filename);
    VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

    // JEShader - abstract base class
    class JEShader {
    protected:
        const std::string m_vertPath;
        const std::string m_fragPath;

    public:
        JEShader(const std::string& vertPath, const std::string& fragPath) : m_vertPath(vertPath), m_fragPath(fragPath) {}
        virtual ~JEShader() = default;

        virtual void Cleanup() = 0;
        virtual void UpdateUniformBuffers(VkDevice device, uint32_t currentImage) = 0;
    };

    // TODO: support D3D shaders via inheritance (JED3DXShader class)

    class JEVulkanShader : public JEShader {
    protected:
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        const VkDevice m_device;

        // Creation functions
        virtual void CreateDescriptorSetLayouts(VkDevice device, uint32_t numSourceTextures, uint32_t numUniformBuffers, uint32_t numStorageBuffers) {
            for (uint32_t s = 0; s < m_descriptorSetLayouts.size(); ++s) {
                std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

                if (s == 0) {
                    for (uint32_t i = 0; i < numUniformBuffers; ++i) {
                        VkDescriptorSetLayoutBinding layoutBinding = {};
                        layoutBinding.binding = i;
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
                        layoutBinding.descriptorCount = 1;
                        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                        layoutBinding.pImmutableSamplers = nullptr;
                        setLayoutBindings.push_back(layoutBinding);
                    }

                    for (uint32_t i = 0; i < numSourceTextures; ++i) {
                        VkDescriptorSetLayoutBinding layoutBinding = {};
                        layoutBinding.binding = i + numUniformBuffers;
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                        layoutBinding.descriptorCount = 1;
                        layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                        layoutBinding.pImmutableSamplers = nullptr;
                        setLayoutBindings.push_back(layoutBinding);
                    }
                } else {
                    for (uint32_t i = 0; i < numStorageBuffers; ++i) {
                        VkDescriptorSetLayoutBinding layoutBinding = {};
                        layoutBinding.binding = i;
                        layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                        layoutBinding.descriptorCount = 1;
                        layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
                        layoutBinding.pImmutableSamplers = nullptr;
                        setLayoutBindings.push_back(layoutBinding);
                    }
                }

                if (setLayoutBindings.size() > 0) {
                    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                    layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                    layoutInfo.pBindings = setLayoutBindings.data();

                    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayouts[s]) != VK_SUCCESS) {
                        throw std::runtime_error("failed to create descriptor set layout!");
                    }
                }
            }
        }
        virtual void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) = 0;

    public:
        JEVulkanShader(VkDevice device, const std::string& vertPath, const std::string& fragPath) :
            JEShader(vertPath, fragPath), m_device(device) {
            m_descriptorSetLayouts = std::vector<VkDescriptorSetLayout>(2, VK_NULL_HANDLE);
        }
        virtual ~JEVulkanShader() {}

        void Cleanup() override {
            if (m_pipelineLayout != VK_NULL_HANDLE) {
                vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
            }
            if (m_graphicsPipeline != VK_NULL_HANDLE) {
                vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);
            }
            for (uint32_t i = 0; i < m_descriptorSetLayouts.size(); ++i) {
                if (m_descriptorSetLayouts[i] != VK_NULL_HANDLE) {
                    vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayouts[i], nullptr);
                }
            }
        }

        virtual void UpdateUniformBuffers(VkDevice device, uint32_t currentImage) override {}

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
        VkPipelineLayout GetPipelineLayout() const {
            return m_pipelineLayout;
        }
        VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index) const {
            // TODO: error check?
            return m_descriptorSetLayouts[index];
        }
    };

    // JEShadowShader - shadow pass variant
    class JEShadowShader : public JEVulkanShader {
    private:
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        JEShadowShader() = delete;
        JEShadowShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, VkDevice device, VkPhysicalDevice physicalDevice,
            VkExtent2D extent, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) :
            JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);

            CreateDescriptorSetLayouts(device, numSourceTextures, 0, 1);
            CreateGraphicsPipeline(device, vertShaderModule, VK_NULL_HANDLE, extent, renderPass, materialComponent);
        }

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    class JEDeferredGeometryShader : public JEVulkanShader {
    private:
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        JEDeferredGeometryShader() = delete;
        JEDeferredGeometryShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, uint32_t numUniformBuffers,
            VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanSwapChain& swapChain, VkRenderPass renderPass,
            const std::string& vertPath, const std::string& fragPath) : JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 1);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    // JEDeferredShader - deferred lighting variant
    class JEDeferredShader : public JEVulkanShader {
    private:
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        JEDeferredShader() = delete;
        JEDeferredShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, uint32_t numUniformBuffers, VkDevice device, VkPhysicalDevice physicalDevice,
                         const JEVulkanSwapChain& swapChain, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) :
                         JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            m_descriptorSetLayouts = std::vector<VkDescriptorSetLayout>(1, VK_NULL_HANDLE);
            CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 0);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }
    };

    // JEForwardShader - forward shading
    class JEForwardShader : public JEVulkanShader {
    private:
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        JEForwardShader() = delete;
        JEForwardShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, uint32_t numUniformBuffers, VkDevice device, VkPhysicalDevice physicalDevice,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) :
            JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 1);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    class JEForwardTranslucentShader : public JEVulkanShader {
    private:
        bool m_oit;

        void CreateDescriptorSetLayouts(VkDevice device, uint32_t numSourceTextures, uint32_t numUniformBuffers, uint32_t numStorageBuffers) override {
            m_descriptorSetLayouts.push_back(VK_NULL_HANDLE);
            
            std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings;

            for (uint32_t i = 0; i < numStorageBuffers; ++i) {
                VkDescriptorSetLayoutBinding layoutBinding = {};
                layoutBinding.binding = i;
                layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
                layoutBinding.descriptorCount = 1;
                layoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                layoutBinding.pImmutableSamplers = nullptr;
                setLayoutBindings.push_back(layoutBinding);
            }

            if (setLayoutBindings.size() > 0) {
                VkDescriptorSetLayoutCreateInfo layoutInfo = {};
                layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
                layoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
                layoutInfo.pBindings = setLayoutBindings.data();

                // TODO: change this hard-coded index
                if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayouts[2]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        JEForwardTranslucentShader() = delete;
        JEForwardTranslucentShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, uint32_t numUniformBuffers, VkDevice device, VkPhysicalDevice physicalDevice,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass, bool enableOIT, const std::string& vertPath, const std::string& fragPath) :
            JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            m_oit = enableOIT;
            JEVulkanShader::CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 1);
            if (m_oit) {
                CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 4);
            }
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };
}
