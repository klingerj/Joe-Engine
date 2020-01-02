#pragma once

#include <fstream>
#include <vector>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"

#include "VulkanSwapChain.h"
#include "VulkanRenderingTypes.h"
#include "TextureLibrary.h"
#include "../Scene/Camera.h"
#include "../Components/Material/MaterialComponent.h"

namespace JoeEngine {
    //! View-projection matrix struct data for push constants.
    struct JE_PushConst_ViewProj {
        glm::mat4 viewProj;
    };

    //! Inverse view and inverse projection matrix struct data for push constants.
    struct JEUBO_ViewProj_Inv {
        glm::mat4 invProj;
        glm::mat4 invView;
    };

    //! Model matrix struct data for push constants.
    struct JE_PushConst_ModelMat {
        glm::mat4 model;
    };

    //! Read a shader from a file source path.
    /*!
      \param filename the file path.
      \return list of characters of the shader source.
    */
    std::vector<char> ReadFile(const std::string& filename);

    //! Create a shader module object from shader source code.
    /*!
      \param device the Vulkan logical device.
      \param code the shader source code.
      \return a Vulkan shader module object.
    */
    VkShaderModule CreateShaderModule(VkDevice device, const std::vector<char>& code);

    //! The JEShader class.
    /*!
      Abstract base class for derived shader classes. Meant to be extended by shader classes for multiple graphics APIs,
      e.g., Vulkan, D3DX, OpenGL, etc.
    */
    class JEShader {
    protected:
        //! Path to vertex shader source file.
        const std::string m_vertPath;

        //! Path to fragment shader source file.
        const std::string m_fragPath;

    public:
        //! Delete constructor (deleted).
        JEShader() = delete;

        //! Constructor.
        JEShader(const std::string& vertPath, const std::string& fragPath) : m_vertPath(vertPath), m_fragPath(fragPath) {}

        //! Destructor (default) (virtual).
        virtual ~JEShader() = default;
        
        //! Purely virtual cleanup function.
        virtual void Cleanup() = 0;
        
        //! Purely virtual function for updating shader buffers.
        /*!
          \param device the Vulkan logical device.
          \param currentImage the currently active swap chain image.
        */
        virtual void UpdateUniformBuffers(VkDevice device, uint32_t currentImage) = 0;
    };

    // TODO: support D3D shaders via inheritance (JED3DXShader class)

    //! The JEVulkanShader
    /*!
      Class that directly extends the JEShader class. Meant to be extended by all specific Vulkan shader class.
      Stores Vulkan API specific data.
    */
    class JEVulkanShader : public JEShader {
    protected:
        //! Graphics pipeline object.
        VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;

        //! Pipeline layout object.
        VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

        //! List of descriptor set layouts (number of elements dependent on the purpose of the derived shader).
        std::vector<VkDescriptorSetLayout> m_descriptorSetLayouts;
        
        //! Reference to the Vulkan logical device.
        const VkDevice m_device;

        // Creation functions

        //! Base class version of descriptor set layout creation.
        /*!
          \param device the Vulkan logical device.
          \param numSourceTextures the number of uniform sampler textures.
          \param numUniformBuffers the number of uniform data buffers.
          \param numStorageBuffers the number of shader storage data buffers.
        */
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

        //! Create graphics pipeline function.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        virtual void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) = 0;

    public:
        //! Default constructor (deleted).
        JEVulkanShader() = delete;

        //! Constructor.
        JEVulkanShader(VkDevice device, const std::string& vertPath, const std::string& fragPath) :
            JEShader(vertPath, fragPath), m_device(device) {
            m_descriptorSetLayouts = std::vector<VkDescriptorSetLayout>(2, VK_NULL_HANDLE);
        }

        //! Destructor (default) (deleted).
        virtual ~JEVulkanShader() = default;

        //! Vulkan API-specific cleanup function.
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

        //! Default shader data buffer update function.
        /*!
          \param device the Vulkan logical device.
          \param currentImage the currently active swap chain image.
        */
        virtual void UpdateUniformBuffers(VkDevice device, uint32_t currentImage) override {}

        //! Get graphics pipeline object.
        //! \return the stored graphics pipeline object.
        VkPipeline GetPipeline() const {
            return m_graphicsPipeline;
        }

        //! Get graphics pipeline layout object.
        //! \return the stored graphics pipeline layout object.
        VkPipelineLayout GetPipelineLayout() const {
            return m_pipelineLayout;
        }

        //! Get descriptor set layout object.
        /*!
          \param index the specific descriptor set layout to return.
          \return the specific descriptor set layout.
        */
        VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index) const {
            // TODO: error check?
            return m_descriptorSetLayouts[index];
        }
    };

    //! The JEShadowShader
    /*!
      Vulkan shader class variant for shadow pass rendering.
      \sa JEShader, JEVulkanShader
    */
    class JEShadowShader : public JEVulkanShader {
    private:
        //! Create graphics pipeline overridden implementation.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (deleted).
        JEShadowShader() = delete;

        //! Constructor.
        /*!
          Loads the specified shader files from the source paths and creates the Vulkan objects based on the material properties.
        */
        JEShadowShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, VkDevice device, VkPhysicalDevice physicalDevice,
            VkExtent2D extent, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) :
            JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);

            CreateDescriptorSetLayouts(device, numSourceTextures, 0, 1);
            CreateGraphicsPipeline(device, vertShaderModule, VK_NULL_HANDLE, extent, renderPass, materialComponent);
        }

        //! Bind the view-projection matrix push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param viewProj the view projection matrix to provide to the shader.
        */
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;

        //! Bind the instanced data push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param instancedData per-geometry-instance data to provide to the shader.
        */
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    //! The JEDeferredGeometryShader
    /*!
      The shader that writes to the G-buffers for deferred rendering.
      \sa JEShader, JEVulkanShader
    */
    class JEDeferredGeometryShader : public JEVulkanShader {
    private:
        //! Create graphics pipeline.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (deleted).
        JEDeferredGeometryShader() = delete;

        //! Constructor.
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

        //! Bind the view-projection matrix push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param viewProj the view projection matrix to provide to the shader.
        */
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;

        //! Bind the instanced data push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param instancedData per-geometry-instance data to provide to the shader.
        */
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    //! The JEDeferredShader class.
    /*!
      Shader for shading objects given multiple G-buffers.
      Intended to be used with a screen-space triangle/quad.
      \sa JEShader, JEVulkanShader
    */
    class JEDeferredShader : public JEVulkanShader {
    private:
        //! Create graphics pipeline.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (delete).
        JEDeferredShader() = delete;

        //! Constructor.
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

    //! The JEForwardShader class.
    /*!
      Shader for typical forward shading.
      Currently used for rendering of translucent objects.
      \sa JEShader, JEVulkanShader
    */
    class JEForwardShader : public JEVulkanShader {
    private:
        //! Create graphics pipeline.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (delete).
        JEForwardShader() = delete;

        //! Constructor.
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

        //! Bind the view-projection matrix push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param viewProj the view projection matrix to provide to the shader.
        */
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;

        //! Bind the instanced data push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param instancedData per-geometry-instance data to provide to the shader.
        */
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    //! The JEPointsShader
    /*!
      Shader for rendering points meshes with a single texture.
      \sa JEShader, JEVulkanShader
    */
    class JEPointsShader : public JEVulkanShader {
    private:
        //! Create graphics pipeline.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;
        
    public:
        //! Default constructor (deleted).
        JEPointsShader() = delete;
        
        //! Constructor.
        JEPointsShader(const MaterialComponent& materialComponent, uint32_t numSourceTextures, uint32_t numUniformBuffers,
            VkDevice device, VkPhysicalDevice physicalDevice, const JEVulkanSwapChain& swapChain, VkRenderPass renderPass,
            const std::string& vertPath, const std::string& fragPath) : JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateDescriptorSetLayouts(device, numSourceTextures, numUniformBuffers, 0);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }

        //! Bind the view-projection matrix push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param viewProj the view projection matrix to provide to the shader.
        */
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;
    };

    //! The JEForwardTranslucentShader class.
    /*!
      Shader for rendering translucent meshes, optionally with OIT enabled.
      \sa JEShader, JEVulkanShader
    */
    class JEForwardTranslucentShader : public JEVulkanShader {
    private:
        //! Flag indicating that this shader is intended to be used with OIT.
        bool m_oit;

        //! Derived class version of descriptor set layout creation.
        /*!
          \param device the Vulkan logical device.
          \param numSourceTextures the number of uniform sampler textures.
          \param numUniformBuffers the number of uniform data buffers.
          \param numStorageBuffers the number of shader storage data buffers.
        */
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

        //! Create graphics pipeline.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (deleted).
        JEForwardTranslucentShader() = delete;
        
        //! Constructor.
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

        //! Bind the view-projection matrix push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param viewProj the view projection matrix to provide to the shader.
        */
        void BindPushConstants_ViewProj(VkCommandBuffer commandBuffer, const glm::mat4& viewProj) const;

        //! Bind the instanced data push constant.
        /*!
          \param commandBuffer the command buffer to record the bind command to.
          \param instancedData per-geometry-instance data to provide to the shader.
        */
        void BindPushConstants_InstancedData(VkCommandBuffer commandBuffer, const std::array<uint32_t, 4>& instancedData) const;
    };

    //! The JEOITSortShader class.
    /*!
      Shader for compositing a GPU linked-list of translucent fragments into a single, final, blended fragment color.
      \sa JEShader, JEVulkanShader
    */
    class JEOITSortShader : public JEVulkanShader {
    private:
        //! Derived class version of descriptor set layout creation.
        /*!
          \param device the Vulkan logical device.
          \param numSourceTextures the number of uniform sampler textures.
          \param numUniformBuffers the number of uniform data buffers.
          \param numStorageBuffers the number of shader storage data buffers.
        */
        void CreateDescriptorSetLayouts(VkDevice device, uint32_t numSourceTextures, uint32_t numUniformBuffers, uint32_t numStorageBuffers) override {
            m_descriptorSetLayouts = { VK_NULL_HANDLE };

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
                if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_descriptorSetLayouts[0]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create descriptor set layout!");
                }
            }
        }

        //! Create graphics pipeline overridden implementation.
        /*!
          \param device the Vulkan logical device.
          \param vertShaderModule the vertex shader module object.
          \param fragShaderModule the fragment shader module object.
          \param frameExtent the window/frame dimensions.
          \param renderPass the intended Vulkan render pass object for the shader.
          \param materialComponent the material properties for the shader.
        */
        void CreateGraphicsPipeline(VkDevice device, VkShaderModule vertShaderModule, VkShaderModule fragShaderModule,
            VkExtent2D frameExtent, VkRenderPass renderPass, const MaterialComponent& materialComponent) override;

    public:
        //! Default constructor (deleted).
        JEOITSortShader() = delete;

        //! Constructor.
        JEOITSortShader(const MaterialComponent& materialComponent, VkDevice device, VkPhysicalDevice physicalDevice,
            const JEVulkanSwapChain& swapChain, VkRenderPass renderPass, const std::string& vertPath, const std::string& fragPath) :
            JEVulkanShader(device, vertPath, fragPath) {
            auto vertShaderCode = ReadFile(m_vertPath);
            auto fragShaderCode = ReadFile(m_fragPath);
            // Create shader modules
            VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);
            VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

            uint32_t numSwapChainImages = swapChain.GetImageViews().size();
            CreateDescriptorSetLayouts(device, 0, 0, 4);
            CreateGraphicsPipeline(device, vertShaderModule, fragShaderModule, swapChain.GetExtent(), renderPass, materialComponent);
        }
    };
}
