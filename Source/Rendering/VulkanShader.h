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

    // JEForwardShader - forward shadiung
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
            CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 0);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }
    };

    // *************************** end new shader style
    // Forward shader

    class JEVulkanForwardShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorSet m_descriptorSet;

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass);
        void CreateDescriptorPool(VkDevice device);
        void CreateDescriptorSetLayout(VkDevice device);
        void CreateDescriptorSets(VkDevice device, const JETexture& texture);
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device);

    public:
        JEVulkanForwardShader() {}
        JEVulkanForwardShader(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& swapChain, VkRenderPass renderPass,
            const JETexture& texture, const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);
            auto fragShaderCode = ReadFile(fragShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);
            
            CreateUniformBuffers(physicalDevice, device);
            CreateDescriptorSetLayout(device);
            CreateDescriptorPool(device);
            CreateDescriptorSets(device, texture);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
        }

        ~JEVulkanForwardShader() {}

        void Cleanup(VkDevice device);

        void UpdateUniformBuffers(VkDevice device);
        void BindDescriptorSets(VkCommandBuffer commandBuffer);

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj);
        void BindPushConstants_ModelMatrix(VkCommandBuffer commandBuffer, const glm::mat4& modelMat);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };

    // Flat shader

    class JEVulkanFlatShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D extent, VkRenderPass renderPass);
        //void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages);
        //void CreateDescriptorSetLayout(VkDevice device);
        //void CreateDescriptorSets(VkDevice device, const JEPostProcessingPass& postProcessingPass, VkImageView postImageView, uint32_t numSwapChainImages);
        //void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages);

    public:
        JEVulkanFlatShader() {}
        JEVulkanFlatShader(VkPhysicalDevice physicalDevice, VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
            const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);
            auto fragShaderCode = ReadFile(fragShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            //uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            //CreateUniformBuffers(physicalDevice, device, numSwapChainImages);
            //CreateDescriptorSetLayout(device);
            //CreateDescriptorPool(device, numSwapChainImages);
            //CreateDescriptorSets(device, postProcessingPass, postImageView, numSwapChainImages);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, extent, renderPass);
        }

        ~JEVulkanFlatShader() {}

        void Cleanup(VkDevice device);

        //void UpdateUniformBuffers(VkDevice device, uint32_t currentImage);
        //void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t descriptorSetIndex);
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj);
        void BindPushConstants_ModelMatrix(VkCommandBuffer commandBuffer, const glm::mat4& modelMat);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };

    // Post Processing Shader: Draws a meshes with a texture uniform.

    class JEVulkanPostProcessShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass);
        void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages);
        void CreateDescriptorSetLayout(VkDevice device);
        void CreateDescriptorSets(VkDevice device, const JEPostProcessingPass& postProcessingPass, VkImageView postImageView, uint32_t numSwapChainImages);
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages);

    public:
        JEVulkanPostProcessShader() {}
        JEVulkanPostProcessShader(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& swapChain, const JEPostProcessingPass& postProcessingPass,
                                  VkImageView postImageView, const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);
            auto fragShaderCode = ReadFile(fragShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateUniformBuffers(physicalDevice, device, numSwapChainImages);
            CreateDescriptorSetLayout(device);
            CreateDescriptorPool(device, numSwapChainImages);
            CreateDescriptorSets(device, postProcessingPass, postImageView, numSwapChainImages);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, postProcessingPass.renderPass);
        }

        ~JEVulkanPostProcessShader() {}

        void Cleanup(VkDevice device);

        void UpdateUniformBuffers(VkDevice device, uint32_t currentImage);
        void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t descriptorSetIndex);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };

    // Shadow pass: Render to depth from the perspective of a light source

    class JEVulkanShadowPassShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorSet m_descriptorSet;

        // Buffers
        /*VkBuffer m_uniformBuffers_ViewProj;
        VkDeviceMemory m_uniformBuffersMemory_ViewProj;
        JE_PushConst_ModelMat m_ubo_ModelMat;
        VkBuffer m_uniformBuffers_Model;
        VkDeviceMemory m_uniformBuffersMemory_Model;*/

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkExtent2D extent, VkRenderPass renderPass);
        void CreateDescriptorPool(VkDevice device);
        void CreateDescriptorSetLayout(VkDevice device);
        void CreateDescriptorSets(VkDevice device);
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device);

    public:
        JEVulkanShadowPassShader() {}
        JEVulkanShadowPassShader(VkPhysicalDevice physicalDevice, VkDevice device, VkRenderPass renderPass, VkExtent2D extent,
                                 const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);

            CreateUniformBuffers(physicalDevice, device);
            CreateDescriptorSetLayout(device);
            CreateDescriptorPool(device);
            CreateDescriptorSets(device);
            CreateGraphicsPipeline(device, vertShaderModule, extent, renderPass);
        }

        ~JEVulkanShadowPassShader() {}

        void Cleanup(VkDevice device);

        void UpdateUniformBuffers(VkDevice device);
        void BindDescriptorSets(VkCommandBuffer commandBuffer);

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj);
        void BindPushConstants_ModelMatrix(VkCommandBuffer commandBuffer, const glm::mat4& modelMat);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };

    // Deferred Geometry Pass Shader: Renders meshes to g-buffers

    class JEVulkanDeferredPassGeometryShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        VkDescriptorSet m_descriptorSet;

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass);
        void CreateDescriptorPool(VkDevice device);
        void CreateDescriptorSetLayout(VkDevice device);
        void CreateDescriptorSets(VkDevice device, const JETexture& texture);
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device);

    public:
        JEVulkanDeferredPassGeometryShader() {}
        JEVulkanDeferredPassGeometryShader(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& swapChain, VkRenderPass renderPass,
                                           const JETexture& texture, const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);
            auto fragShaderCode = ReadFile(fragShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateUniformBuffers(physicalDevice, device);
            CreateDescriptorSetLayout(device);
            CreateDescriptorPool(device);
            CreateDescriptorSets(device, texture);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
        }

        ~JEVulkanDeferredPassGeometryShader() {}

        void Cleanup(VkDevice device);

        void UpdateUniformBuffers(VkDevice device);
        void BindDescriptorSets(VkCommandBuffer commandBuffer);

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj);
        void BindPushConstants_ModelMatrix(VkCommandBuffer commandBuffer, const glm::mat4& modelMat);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };

    // Deferred Lighting Pass: Renders a scene using G-buffers

    class JEVulkanDeferredPassLightingShader {
    private:
        VkPipeline m_graphicsPipeline;
        VkPipelineLayout m_pipelineLayout;
        VkDescriptorPool m_descriptorPool;
        VkDescriptorSetLayout m_descriptorSetLayout;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // Buffers
        std::vector<VkBuffer> m_uniformBuffers_ViewProj;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory_ViewProj;
        std::vector<VkBuffer> m_uniformBuffers_ViewProj_Shadow;
        std::vector<VkDeviceMemory> m_uniformBuffersMemory_ViewProj_Shadow;

        // Creation functions
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass);
        void CreateDescriptorPool(VkDevice device, uint32_t numSwapChainImages);
        void CreateDescriptorSetLayout(VkDevice device);
        void CreateDescriptorSets(VkDevice device, const JETexture& texture, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass, uint32_t numSwapChainImages);
        void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t numSwapChainImages);

    public:
        JEVulkanDeferredPassLightingShader() {}
        JEVulkanDeferredPassLightingShader(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanSwapChain& swapChain, const JEOffscreenShadowPass& shadowPass, const JEOffscreenDeferredPass& deferredPass,
            VkRenderPass renderPass, const JETexture& texture, const std::string& vertShader, const std::string& fragShader) {
            // Read in shader code
            auto vertShaderCode = ReadFile(vertShader);
            auto fragShaderCode = ReadFile(fragShader);

            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateUniformBuffers(physicalDevice, device, numSwapChainImages);
            CreateDescriptorSetLayout(device);
            CreateDescriptorPool(device, numSwapChainImages);
            CreateDescriptorSets(device, texture, shadowPass, deferredPass, numSwapChainImages);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain, renderPass);
        }

        ~JEVulkanDeferredPassLightingShader() {}

        void Cleanup(VkDevice device);

        void UpdateUniformBuffers(VkDevice device, uint32_t currentImage, const JECamera& camera, const JECamera& shadowCamera);
        void BindDescriptorSets(VkCommandBuffer commandBuffer, uint32_t descriptorSetIndex);

        // Getters
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }
    };
}
