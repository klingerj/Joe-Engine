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
    };

    //JEDeferredShader - deferred lighting variant
    class JEDeferredShader : JEShader {
    private:
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
        VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
        VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
        std::vector<VkDescriptorSet> m_descriptorSets;

        // Creation functions
        //void CreateUniformBuffers(VkPhysicalDevice physicalDevice, VkDevice device);
        void CreateDescriptorSetLayout(VkDevice device, const MaterialComponent& materialComponent);
        void CreateDescriptorPool(VkDevice device, const MaterialComponent& materialComponent, uint32_t numSwapChainImages);
        void CreateDescriptorSets(VkDevice device, const MaterialComponent& materialComponent, const JETextureLibrary& textures,
                                  std::vector<JEOffscreenShadowPass> shadowPasses, JEOffscreenDeferredPass deferredGeometryPass, uint32_t numSwapChainImages);
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
                                    VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent);

    public:
        JEDeferredShader() = delete;
        JEDeferredShader(const MaterialComponent& materialComponent, const JETextureLibrary& textures, std::vector<JEOffscreenShadowPass> shadowPasses,
                         JEOffscreenDeferredPass deferredGeometryPass, VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanSwapChain& swapChain,
                         VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) : JEShader(vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            //CreateUniformBuffers(physicalDevice, device);
            CreateDescriptorSetLayout(device, materialComponent);
            CreateDescriptorPool(device, materialComponent, numSwapChainImages);
            CreateDescriptorSets(device, materialComponent, textures, shadowPasses, deferredGeometryPass, numSwapChainImages);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }

        void Cleanup(VkDevice device);

        //void UpdateUniformBuffers(VkDevice device);
        void BindDescriptorSets(VkCommandBuffer commandBuffer);

        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj);
        void BindPushConstants_ModelMatrix(VkCommandBuffer commandBuffer, const glm::mat4& modelMat);
    };



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
