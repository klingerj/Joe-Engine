#include <map>
#include <vector>
#include <iostream>

#include "JoeEngineConfig.h"
#include "../EngineInstance.h"
#include "../Utils/ScopedTimer.h"
#include "VulkanRenderer.h"
#include "../Scene/SceneManager.h"
#include "../EngineInstance.h"

namespace JoeEngine {
    static void JEFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto& renderer = reinterpret_cast<JEEngineInstance*>(glfwGetWindowUserPointer(window))->GetRenderSubsystem();
        renderer.FramebufferResized();
    }

    void JEVulkanRenderer::Initialize(RendererSettings rendererSettings, JESceneManager* sceneManager, JEEngineInstance* engineInstance) {
        m_useDeferred = rendererSettings & RendererSettings::EnableDeferred;
        m_enableOIT = rendererSettings & RendererSettings::EnableOIT;

        m_engineInstance = engineInstance;
        m_sceneManager = sceneManager;

        // Window (GLFW)
        m_vulkanWindow.Initialize(m_width, m_height, "VulkanWindow", m_instance);

        /// Vulkan setup

        // Vulkan Instance
        CreateVulkanInstance();

        // Validation Layers
        m_vulkanValidationLayers.SetupDebugCallback(m_instance);

        // Window surface
        m_vulkanWindow.SetupVulkanSurface(m_instance);
        m_vulkanWindow.SetFrameBufferCallback(JEFramebufferResizeCallback);

        // Devices
        PickPhysicalDevice();
        CreateLogicalDevice();

        // Swap Chain
        m_vulkanSwapChain.Create(m_physicalDevice, m_device, m_vulkanWindow, m_width, m_height);

        // Command Pool
        CreateCommandPool();

        // Mesh Buffers
        m_meshBufferManager.Initialize(m_physicalDevice, m_device, m_commandPool, m_graphicsQueue);

        // Create deferred lighting pass framebuffer attachments

        // Add the post processing passes
        /*JEPostProcessingPass p;
        p.shaderIndex = 0;
        m_postProcessingPasses.push_back(p);
        p.shaderIndex = 1;
        m_postProcessingPasses.push_back(p);*/

        // Create the post processing pass(es)
        CreatePostProcessingPassResources();

        // Shadow Pass(es)
        CreateShadowPassResources();

        // Deferred Rendering Passes
        CreateDeferredPassGeometryResources();
        CreateDeferredPassLightingResources();

        // Forward Rendering Pass
        CreateForwardPassResources();

        // Create the swap chain framebuffers
        CreateSwapChainFramebuffers();

        // Create the shadow pass shader
        m_shadowShaderID = m_shaderManager.CreateShader(m_device, m_physicalDevice, m_vulkanSwapChain, MaterialComponent(), 0,
            m_shadowPass.renderPass, JE_SHADER_DIR + "vert_shadow.spv", "", SHADOW);
        m_shadowModelMatrixDescriptorID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
            {}, {}, {}, { JE_NUM_ENTITIES * sizeof(glm::mat4) },
            ((JEVulkanShader*)m_shaderManager.GetShaderAt(m_shadowShaderID))->GetDescriptorSetLayout(1), SHADOW);

        // Forward pass ssbo descriptor
        m_forwardModelMatrixDescriptorID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
            {}, {}, {}, { JE_NUM_ENTITIES * sizeof(glm::mat4) },
            ((JEVulkanShader*)m_shaderManager.GetShaderAt(m_shadowShaderID))->GetDescriptorSetLayout(1), FORWARD);
        // NOTE: that shader ID is obviously wrong, but the descriptor set layout would be the same for any forward shader, as it
        // is just a single ssbo for the model matrices.

        // Create the deferred geometry shader
        if (m_useDeferred) {
            MaterialComponent temp;
            // TODO: deal with this hard-coded number
            m_deferredGeometryShaderID = m_shaderManager.CreateShader(m_device, m_physicalDevice, m_vulkanSwapChain, temp, 4,
                m_deferredPass.renderPass, JE_SHADER_DIR + "vert_deferred_geom.spv", JE_SHADER_DIR + "frag_deferred_geom_new.spv", DEFERRED_GEOM);
            m_deferredGeometryModelMatrixDescriptorID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                {}, {}, {}, { JE_NUM_ENTITIES * sizeof(glm::mat4) },
                ((JEVulkanShader*)m_shaderManager.GetShaderAt(m_deferredGeometryShaderID))->GetDescriptorSetLayout(1), DEFERRED_GEOM);

            // TODO: hard-coded number
            CreateShader(temp, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new.spv");
            m_deferredLightingDescriptorID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                { m_deferredPass.color.imageView, m_deferredPass.normal.imageView, m_deferredPass.depth.imageView, m_shadowPass.depth.imageView },
                { m_deferredPass.sampler, m_deferredPass.sampler, m_deferredPass.sampler, m_shadowPass.depthSampler },
                { sizeof(glm::mat4) * 2, sizeof(glm::mat4) }, {},
                ((JEVulkanShader*)m_shaderManager.GetShaderAt(temp.m_shaderID))->GetDescriptorSetLayout(0), DEFERRED);
            // ^ These sizes are camera uniform - invView, invProj, then light viewProj
            // Create non-shadows variant of this shader
            // TODO: store these better
            temp.m_materialSettings = NO_SETTINGS;
            CreateShader(temp, JE_SHADER_DIR + "vert_deferred_lighting.spv", JE_SHADER_DIR + "frag_deferred_lighting_new_no_shadows.spv");
            m_deferredLightingNoShadowsDescriptorID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                { m_deferredPass.color.imageView, m_deferredPass.normal.imageView, m_deferredPass.depth.imageView },
                { m_deferredPass.sampler, m_deferredPass.sampler, m_deferredPass.sampler },
                { sizeof(glm::mat4) * 2 }, {},
                ((JEVulkanShader*)m_shaderManager.GetShaderAt(temp.m_shaderID))->GetDescriptorSetLayout(0), DEFERRED);
            // ^ No shadows, so not light view proj and no shadow map
        }

        m_textureLibraryGlobal.CreateTexture(m_device, m_physicalDevice, m_graphicsQueue, m_commandPool, JE_TEXTURES_DIR + "fallback.png");

        // Command buffers
        CreateShadowCommandBuffer();
        CreateDeferredPassGeometryCommandBuffer();
        CreateDeferredLightingAndPostProcessingCommandBuffer();
        CreateForwardPassCommandBuffer();

        // Sync objects
        CreateSemaphoresAndFences();
    }

    void JEVulkanRenderer::Cleanup() {
        CleanupWindowDependentResources();
        m_meshBufferManager.Cleanup();
        m_textureLibraryGlobal.Cleanup(m_device);

        // Cleanup shadow pass
        vkDestroyImage(m_device, m_shadowPass.depth.image, nullptr);
        vkFreeMemory(m_device, m_shadowPass.depth.deviceMemory, nullptr);
        vkDestroyImageView(m_device, m_shadowPass.depth.imageView, nullptr);
        vkDestroySampler(m_device, m_shadowPass.depthSampler, nullptr);
        vkDestroyRenderPass(m_device, m_shadowPass.renderPass, nullptr);
        vkDestroyFramebuffer(m_device, m_shadowPass.framebuffer, nullptr);
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_shadowPass.commandBuffer);
        vkDestroySemaphore(m_device, m_shadowPass.semaphore, nullptr);

        // Deferred Pass - Geometry
        vkDestroySemaphore(m_device, m_deferredPass.semaphore, nullptr);

        for (uint32_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; ++i) {
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);
            vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
        }

        // Forward Pass
        vkDestroySemaphore(m_device, m_forwardPass.semaphore, nullptr);

        vkDestroyCommandPool(m_device, m_commandPool, nullptr);

        vkDestroyDevice(m_device, nullptr);
        if (m_vulkanValidationLayers.AreValidationLayersEnabled()) {
            m_vulkanValidationLayers.DestroyDebugCallback(m_instance);
        }
        m_vulkanWindow.Cleanup(m_instance);
        vkDestroyInstance(m_instance, nullptr);
    }

    std::vector<const char*> JEVulkanRenderer::GetRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = m_vulkanWindow.GetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (m_vulkanValidationLayers.AreValidationLayersEnabled()) {
            extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
        }
        
        return extensions;
    }

    int JEVulkanRenderer::RateDeviceSuitability(VkPhysicalDevice physicalDevice, const JEVulkanWindow& vulkanWindow) {
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

        int score = 0;

        QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, vulkanWindow.GetSurface());
        if (indices.IsComplete()) {
            score += 10000;
        } else {
            return 0;
        }

        // Discrete GPUs have a significant performance advantage
        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 1000;
        }

        // Maximum possible size of textures affects graphics quality
        score += deviceProperties.limits.maxImageDimension2D;

        // Require a certain push constant size
        // TODO: don't hard code
        if (deviceProperties.limits.maxPushConstantsSize < 128) {
            return 0;
        }

        // Application can't function without geometry shaders
        // TODO long term: change device requirements as engine develops
        /*if (!deviceFeatures.geometryShader) {
        return 0;
        }*/

        bool extensionsSupported = m_vulkanSwapChain.CheckDeviceExtensionSupport(physicalDevice);

        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = m_vulkanSwapChain.QuerySwapChainSupport(physicalDevice, vulkanWindow.GetSurface());
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        } else {
            return 0;
        }

        if (!swapChainAdequate) {
            return 0;
        }

        if (!deviceFeatures.samplerAnisotropy) {
            return 0;
        }

        return score;
    }

    void JEVulkanRenderer::CreateVulkanInstance() {
        if (m_vulkanValidationLayers.AreValidationLayersEnabled() && !m_vulkanValidationLayers.CheckValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Joe Engine App";
        appInfo.applicationVersion = VK_MAKE_VERSION(JOE_ENGINE_VERSION_MAJOR, JOE_ENGINE_VERSION_MINOR, 0);
        appInfo.pEngineName = "Joe Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(JOE_ENGINE_VERSION_MAJOR, JOE_ENGINE_VERSION_MINOR, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0; // TODO: change me?

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (m_vulkanValidationLayers.AreValidationLayersEnabled()) {
            const std::vector<const char*>& validationLayers = m_vulkanValidationLayers.GetValidationLayers();
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
            createInfo.ppEnabledLayerNames = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void JEVulkanRenderer::PickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);

        if (deviceCount == 0) {
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());

        // Use an ordered map to automatically sort candidates by increasing score
        std::multimap<int, VkPhysicalDevice> candidates;

        for (const auto& device : devices) {
            int score = RateDeviceSuitability(device, m_vulkanWindow);
            candidates.insert(std::make_pair(score, device));
        }

        // Check if the best candidate is suitable at all
        if (candidates.rbegin()->first > 0) {
            m_physicalDevice = candidates.rbegin()->second;
        } else {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    void JEVulkanRenderer::CreateLogicalDevice() {
        QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice, m_vulkanWindow.GetSurface());

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        //std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = GetQueueCreateInfos(indices);
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { indices.graphicsFamily.value(), indices.presentFamily.value() };

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        createInfo.pEnabledFeatures = &deviceFeatures;

        std::vector<const char*> deviceExtensions = JEVulkanSwapChain::GetDeviceExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        if (m_vulkanValidationLayers.AreValidationLayersEnabled()) {
            const std::vector<const char*>& validationLayers = m_vulkanValidationLayers.GetValidationLayers();
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        } else {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        m_graphicsQueue.GetDeviceQueue(m_device, indices.graphicsFamily.value());
        m_presentationQueue.GetDeviceQueue(m_device, indices.presentFamily.value());
    }

    void JEVulkanRenderer::CreateSwapChainFramebuffers() {
        const std::vector<VkImageView>& swapChainImageViews = m_vulkanSwapChain.GetImageViews();
        m_swapChainFramebuffers.resize(swapChainImageViews.size());

        for (uint32_t i = 0; i < swapChainImageViews.size(); ++i) {
            VkExtent2D extent = m_vulkanSwapChain.GetExtent();

            VkFramebufferCreateInfo framebufferInfo = {};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.width = extent.width;
            framebufferInfo.height = extent.height;
            framebufferInfo.layers = 1;

            if (m_postProcessingPasses.size() > 0) {
                framebufferInfo.renderPass = m_postProcessingPasses[m_postProcessingPasses.size() - 1].renderPass;
            } else {
                if (m_useDeferred) {
                    framebufferInfo.renderPass = m_renderPass_deferredLighting;
                } else {
                    framebufferInfo.renderPass = m_forwardPass.renderPass;
                }
            }

            if (m_useDeferred) {
                std::array<VkImageView, 1> attachments = { swapChainImageViews[i] };
                framebufferInfo.attachmentCount = attachments.size();
                framebufferInfo.pAttachments = attachments.data();
                if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            } else {
                std::array<VkImageView, 2> attachments = { swapChainImageViews[i], m_forwardPass.depth.imageView };
                framebufferInfo.attachmentCount = attachments.size();
                framebufferInfo.pAttachments = attachments.data();
                if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to create framebuffer!");
                }
            }
        }
    }

    void JEVulkanRenderer::CreateCommandPool() {
        QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(m_physicalDevice, m_vulkanWindow.GetSurface());

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS) {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void JEVulkanRenderer::CreateDeferredLightingAndPostProcessingCommandBuffer() {
        m_commandBuffers.resize(m_swapChainFramebuffers.size());

        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t)m_commandBuffers.size();

        if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void JEVulkanRenderer::CreateSemaphoresAndFences() {
        m_imageAvailableSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(m_MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(m_MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo = {};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (uint32_t i = 0; i < m_MAX_FRAMES_IN_FLIGHT; ++i) {
            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create semaphores for a frame!");
            }
        }
    }

    void JEVulkanRenderer::CreateFramebufferAttachment(JEFramebufferAttachment& attachment, VkExtent2D extent, VkImageUsageFlagBits usageBits, VkFormat format) {
        VkImageAspectFlags aspectMask = 0;
        VkImageLayout imageLayout;
        if (usageBits & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) {
            aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }
        if (usageBits & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }
        CreateImage(m_physicalDevice, m_device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, usageBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.deviceMemory);
        attachment.imageView = CreateImageView(m_device, attachment.image, format, aspectMask);
        TransitionImageLayout(m_device, m_commandPool, m_graphicsQueue, attachment.image, format, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
    }

    void JEVulkanRenderer::CreateFramebufferAttachmentSampler(VkSampler& sampler) {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = 1.0f;

        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    /// Renderer Functions

    /*void JEVulkanRenderer::CreateTextures() {
        JETexture t = JETexture(m_device, m_physicalDevice, m_graphicsQueue, m_commandPool, JE_TEXTURES_DIR + "ducreux.jpg");
        m_textures.push_back(t);
    }*/

    /*void JEVulkanRenderer::CleanupTextures() {
        /*for (JETexture t : m_textures) {
            t.Cleanup(m_device);
        }
        m_textures.clear();

        m_textureLibraryGlobal.Cleanup(m_device);
    }*/

    void JEVulkanRenderer::CleanupShaders() {
        m_shaderManager.Cleanup();
    }

    void JEVulkanRenderer::UpdateShaderBuffers(const std::vector<MaterialComponent>& materialComponents,
        const std::vector<glm::mat4>& transforms, const std::vector<glm::mat4>& transformsSorted, uint32_t imageIndex) {
        
        m_shaderManager.UpdateBuffers(m_shadowModelMatrixDescriptorID, imageIndex, {}, {},
            { transforms.data() }, { (uint32_t)(transforms.size() * sizeof(glm::mat4)) });
        m_shaderManager.UpdateBuffers(m_deferredGeometryModelMatrixDescriptorID, imageIndex, {}, {},
            { transformsSorted.data() }, { (uint32_t)(transformsSorted.size() * sizeof(glm::mat4)) });
        m_shaderManager.UpdateBuffers(m_forwardModelMatrixDescriptorID, imageIndex, {}, {},
            { transformsSorted.data() }, { (uint32_t)(transformsSorted.size() * sizeof(glm::mat4)) });

        if (m_useDeferred) {
            // Add camera inv view/proj matrices as uniforms
            //std::array<glm::mat4, 2> uniformInvViewProjData = { m_sceneManager->m_camera.GetInvProj(), m_sceneManager->m_camera.GetInvView() };
            const std::array<glm::mat4, 2> uniformInvViewProjData = { glm::inverse(m_sceneManager->m_camera.GetProj()), glm::inverse(m_sceneManager->m_camera.GetView()) };

            // Add light viewProj matrix (TODO: matrices) as uniforms
            const std::array<glm::mat4, 1> uniformLightData = { m_sceneManager->m_shadowCamera.GetOrthoViewProj() };

            std::vector<const void*> buffers;
            std::vector<uint32_t> sizes;

            buffers.push_back(uniformInvViewProjData.data());
            buffers.push_back(uniformLightData.data());
            //buffers.insert(buffers.end(), materialComponents[i].m_uniformData.begin(), materialComponents[i].m_uniformData.end());

            sizes.push_back(sizeof(glm::mat4) * uniformInvViewProjData.size());
            sizes.push_back(sizeof(glm::mat4) * uniformLightData.size());
            //sizes.insert(sizes.end(), materialComponents[i].m_uniformDataSizes.begin(), materialComponents[i].m_uniformDataSizes.end());
            //m_shaderManager.UpdateUniformBuffers(materialComponents[i].m_descriptorID, imageIndex, buffers, sizes);
            m_shaderManager.UpdateBuffers(m_deferredLightingDescriptorID, imageIndex, buffers, sizes, {}, {});
        }

        for (uint32_t i = 0; i < materialComponents.size(); ++i) {
            // TODO: Don't do this for every material, ignore duplicates

            if (m_useDeferred) {// Add light viewProj matrix (TODO: matrices) as uniforms
                const std::array<glm::mat4, 1> uniformLightData = { m_sceneManager->m_shadowCamera.GetOrthoViewProj() };

                std::vector<const void*> buffers;
                std::vector<uint32_t> sizes;

                buffers.push_back(uniformLightData.data());
                //buffers.insert(buffers.end(), materialComponents[i].m_uniformData.begin(), materialComponents[i].m_uniformData.end());

                sizes.push_back(sizeof(glm::mat4) * uniformLightData.size());
                //sizes.insert(sizes.end(), materialComponents[i].m_uniformDataSizes.begin(), materialComponents[i].m_uniformDataSizes.end());
                m_shaderManager.UpdateBuffers(materialComponents[i].m_descriptorID, imageIndex, buffers, sizes, {}, {});
            } else {
                // TODO
            }
        }
    }
    
    const std::vector<BoundingBoxData>& JEVulkanRenderer::GetBoundingBoxData() const {
        return m_meshBufferManager.GetBoundingBoxData();
    }

    MeshComponent JEVulkanRenderer::CreateMesh(const std::string& filepath) {
        return m_meshBufferManager.CreateMeshComponent(filepath);
    }

    uint32_t JEVulkanRenderer::CreateTexture(const std::string& filepath) {
        // TODO: specify global/level/etc
        const uint32_t textureID = m_textureLibraryGlobal.CreateTexture(m_device, m_physicalDevice, m_graphicsQueue, m_commandPool, filepath);
        return textureID;
    }

    void JEVulkanRenderer::CreateShader(MaterialComponent& materialComponent, const std::string& vertFilepath,
        const std::string& fragFilepath) {
        std::vector<VkImageView> imageViews;
        std::vector<VkSampler> samplers;

        VkRenderPass renderPass;
        PipelineType type;
        if (m_useDeferred) {
            if (materialComponent.m_renderLayer == TRANSLUCENT) {
                renderPass = m_renderPass_deferredLighting; // m_forwardPass.renderPass;
                type = FORWARD;
            } else {
                renderPass = m_renderPass_deferredLighting;
                type = DEFERRED;
            }
        } else {
            renderPass = m_forwardPass.renderPass;
            type = FORWARD;
        }
        if (type == DEFERRED) {
            imageViews.push_back(m_deferredPass.color.imageView);
            imageViews.push_back(m_deferredPass.normal.imageView);
            imageViews.push_back(m_deferredPass.depth.imageView);
            samplers.push_back(m_deferredPass.sampler);
            samplers.push_back(m_deferredPass.sampler);
            samplers.push_back(m_deferredPass.sampler);
        } else {
            // Forward
            imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texAlbedo));
            imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texRoughness));
            imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texMetallic));
            imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texNormal));
            samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texAlbedo));
            samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texRoughness));
            samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texMetallic));
            samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texNormal));
        }

        // Shadow map(s)
        if (materialComponent.m_materialSettings & RECEIVES_SHADOWS) {
            imageViews.push_back(m_shadowPass.depth.imageView);
            samplers.push_back(m_shadowPass.depthSampler);
        }

        materialComponent.m_shaderID = m_shaderManager.CreateShader(m_device, m_physicalDevice, m_vulkanSwapChain,
            materialComponent, imageViews.size(), renderPass, vertFilepath, fragFilepath, type);
    }

    void JEVulkanRenderer::CreateDescriptor(MaterialComponent& materialComponent) {
        std::vector<VkImageView> imageViews;
        imageViews.reserve(4);
        std::vector<VkSampler> samplers;
        samplers.reserve(4);

        // Source textures
        imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texAlbedo));
        imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texRoughness));
        imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texMetallic));
        imageViews.push_back(m_textureLibraryGlobal.GetImageViewAt(materialComponent.m_texNormal));
        samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texAlbedo));
        samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texRoughness));
        samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texMetallic));
        samplers.push_back(m_textureLibraryGlobal.GetSamplerAt(materialComponent.m_texNormal));

        // Using forward shader
        if (materialComponent.m_renderLayer == TRANSLUCENT) {
            if (materialComponent.m_materialSettings & CASTS_SHADOWS) {
                imageViews.push_back(m_shadowPass.depth.imageView);
                samplers.push_back(m_shadowPass.depthSampler);
            }
        }

        std::vector<uint32_t> uniformBufferSizes;
        if (materialComponent.m_materialSettings & CASTS_SHADOWS) {
            uniformBufferSizes.push_back(sizeof(glm::mat4));
        }

        uint32_t descrID = 0;
        if (m_useDeferred) {
            if (materialComponent.m_renderLayer == TRANSLUCENT) {
                descrID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                    imageViews, samplers, uniformBufferSizes, {},
                    ((JEVulkanShader*)m_shaderManager.GetShaderAt(materialComponent.m_shaderID))->GetDescriptorSetLayout(0), FORWARD);
            } else {
                descrID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                    imageViews, samplers, {}, {},
                    ((JEVulkanShader*)m_shaderManager.GetShaderAt(m_deferredGeometryShaderID))->GetDescriptorSetLayout(0), DEFERRED_GEOM);
            }
        } else {
            descrID = m_shaderManager.CreateDescriptor(m_device, m_physicalDevice, m_vulkanSwapChain,
                imageViews, samplers, uniformBufferSizes, {},
                ((JEVulkanShader*)m_shaderManager.GetShaderAt(materialComponent.m_shaderID))->GetDescriptorSetLayout(0), FORWARD);
        }
        materialComponent.m_descriptorID = descrID;
    }

    void JEVulkanRenderer::DrawBoundingBoxMesh(VkCommandBuffer commandBuffer) {
        const JESingleMesh& boundingBox = m_meshBufferManager.GetBoundingBoxMesh();
        VkBuffer vertexBuffers[] = { boundingBox.vertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, boundingBox.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(boundingBox.indexList.size()), 1, 0, 0, 0);
    }

    void JEVulkanRenderer::DrawScreenSpaceTriMesh(VkCommandBuffer commandBuffer) {
        const JESingleMesh& postTri = m_meshBufferManager.GetScreenSpaceTriMesh();
        VkBuffer vertexBuffers[] = { postTri.vertexBuffer };
        VkDeviceSize offsets[] = { 0 };

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, postTri.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(postTri.indexList.size()), 1, 0, 0, 0);
    }

    void JEVulkanRenderer::DrawMesh(VkCommandBuffer commandBuffer, const MeshComponent& meshComponent) {
        if (meshComponent.GetVertexHandle() == -1 || meshComponent.GetIndexHandle() == -1) {
            return;
        }

        VkBuffer vertexBuffers[] = { m_meshBufferManager.GetVertexBufferAt(meshComponent.GetVertexHandle()) };
        VkDeviceSize offsets[] = { 0 };
        const int idxHandle = meshComponent.GetIndexHandle();

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_meshBufferManager.GetIndexBufferAt(idxHandle), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_meshBufferManager.GetIndexListAt(idxHandle).size()), 1, 0, 0, 0);
    }

    void JEVulkanRenderer::DrawMeshInstanced(VkCommandBuffer commandBuffer, uint32_t numInstances, const MeshComponent& meshComponent) {
       if (meshComponent.GetVertexHandle() == -1 || meshComponent.GetIndexHandle() == -1) {
            return;
        }

        VkBuffer vertexBuffers[] = { m_meshBufferManager.GetVertexBufferAt(meshComponent.GetVertexHandle()) };
        VkDeviceSize offsets[] = { 0 };
        const int idxHandle = meshComponent.GetIndexHandle();

        vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
        vkCmdBindIndexBuffer(commandBuffer, m_meshBufferManager.GetIndexBufferAt(idxHandle), 0, VK_INDEX_TYPE_UINT32);
        vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_meshBufferManager.GetIndexListAt(idxHandle).size()), numInstances, 0, 0, 0);
    }

    void JEVulkanRenderer::DrawShadowPass(/*std vector of JELights*/const std::vector<MeshComponent>& meshComponents, const JECamera& camera) {
        // Reset the command buffer, as we have decided to re-record it
        /*if (vkResetCommandBuffer(m_shadowPass.commandBuffer, VK_COMMAND_BUFFER_RESET_FLAG_BITS_MAX_ENUM) != VK_SUCCESS) {
            throw std::runtime_error("failed to reset shadow pass command buffer!");
        }*/
        // TODO: keep local list of ShadowPassStructs. Push_back to this list during this function for each shadow
        // pass that we do, aka for each light in the vector.
        // Or, do that in a different function so that the render passes and framebuffers are all already created before this function.

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(m_shadowPass.commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording shadow pass command buffer!");
        }
        
        /*TODO: for each light source...*/

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_shadowPass.renderPass;
        renderPassInfo.framebuffer = m_shadowPass.framebuffer;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { m_shadowPass.width, m_shadowPass.height };

        VkClearValue clearValue = { 1.0f, 0 };
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearValue;

        vkCmdBeginRenderPass(m_shadowPass.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        const JEShadowShader* shadowShader = (JEShadowShader*)m_shaderManager.GetShaderAt(m_shadowShaderID);
        vkCmdBindPipeline(m_shadowPass.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shadowShader->GetPipeline());
        m_shaderManager.GetDescriptorAt(m_shadowModelMatrixDescriptorID).BindDescriptorSets(m_shadowPass.commandBuffer, shadowShader->GetPipelineLayout(), 0, 0);
        shadowShader->BindPushConstants_ViewProj(m_shadowPass.commandBuffer, camera.GetOrthoViewProj());

        uint32_t idx = 1;
        uint32_t currStartIdx = 0;
        int currMesh = meshComponents[currStartIdx].GetVertexHandle();

        while (idx <= meshComponents.size()) {
            if (idx == meshComponents.size()) {
                shadowShader->BindPushConstants_InstancedData(m_shadowPass.commandBuffer, {currStartIdx, 0, 0, 0});
                DrawMeshInstanced(m_shadowPass.commandBuffer, idx - currStartIdx, currMesh);
                break;
            }
            if (meshComponents[idx].GetVertexHandle() == currMesh) {
                ++idx;
            } else {
                // Draw instanced mesh using curr material resources
                shadowShader->BindPushConstants_InstancedData(m_shadowPass.commandBuffer, {currStartIdx, 0, 0, 0 });
                DrawMeshInstanced(m_shadowPass.commandBuffer, idx - currStartIdx, currMesh);
                
                currMesh = meshComponents[idx].GetVertexHandle();
                currStartIdx = idx;
            }
        }

        vkCmdEndRenderPass(m_shadowPass.commandBuffer);

        if (vkEndCommandBuffer(m_shadowPass.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to record shadow pass command buffer!");
        }
    }

    void JEVulkanRenderer::DrawMeshComponents(const std::vector<MeshComponent>& meshComponents,
                                              const std::vector<MaterialComponent>& materialComponents,
                                              const JECamera& camera) {
        if (m_useDeferred) {
            /// Construct deferred geometry render pass
            VkCommandBufferBeginInfo beginInfo = {};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
            beginInfo.pInheritanceInfo = nullptr;

            if (vkBeginCommandBuffer(m_deferredPass.commandBuffer, &beginInfo) != VK_SUCCESS) {
                throw std::runtime_error("failed to begin recording deferred geometry pass command buffer!");
            }

            // Begin render pass
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_deferredPass.renderPass;
            renderPassInfo.framebuffer = m_deferredPass.framebuffer;
            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = { m_width, m_height };

            std::array<VkClearValue, 3> clearValues;
            clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
            clearValues[2].depthStencil = { 1.0f, 0 };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_deferredPass.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            JEDeferredGeometryShader* deferredGeomShader = (JEDeferredGeometryShader*)m_shaderManager.GetShaderAt(m_deferredGeometryShaderID);
            vkCmdBindPipeline(m_deferredPass.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, deferredGeomShader->GetPipeline());
            deferredGeomShader->BindPushConstants_ViewProj(m_deferredPass.commandBuffer, camera.GetViewProj());
            m_shaderManager.GetDescriptorAt(m_deferredGeometryModelMatrixDescriptorID).BindDescriptorSets(m_deferredPass.commandBuffer, deferredGeomShader->GetPipelineLayout(), 1, 0);
            
            // for every opaque material component, bind its descriptor each time it changes
            uint32_t idx = 1;
            uint32_t currStartIdx = 0;
            uint32_t currDescriptorID = materialComponents[0].m_descriptorID;
            m_shaderManager.GetDescriptorAt(currDescriptorID).BindDescriptorSets(m_deferredPass.commandBuffer, deferredGeomShader->GetPipelineLayout(), 0, 0);
            int currMesh = meshComponents[currStartIdx].GetVertexHandle();
            
            while (idx <= materialComponents.size()) {
                if (idx == materialComponents.size()) {
                    deferredGeomShader->BindPushConstants_InstancedData(m_deferredPass.commandBuffer, {currStartIdx, 0, 0, 0});
                    DrawMeshInstanced(m_deferredPass.commandBuffer, idx - currStartIdx, currMesh);
                    break;
                }
                if (materialComponents[idx].m_renderLayer == OPAQUE &&
                    materialComponents[idx].m_descriptorID == currDescriptorID &&
                    meshComponents[idx].GetVertexHandle() == currMesh) {
                    ++idx;
                } else {
                    // Draw instanced mesh using curr material resources
                    deferredGeomShader->BindPushConstants_InstancedData(m_deferredPass.commandBuffer, {currStartIdx, 0, 0, 0});
                    DrawMeshInstanced(m_deferredPass.commandBuffer, idx - currStartIdx, currMesh);

                    if (materialComponents[idx].m_renderLayer != OPAQUE) {
                        break;
                    }
                    if (materialComponents[idx].m_descriptorID != currDescriptorID) {
                        m_shaderManager.GetDescriptorAt(materialComponents[idx].m_descriptorID).BindDescriptorSets(m_deferredPass.commandBuffer, deferredGeomShader->GetPipelineLayout(), 0, 0);
                        currDescriptorID = materialComponents[idx].m_descriptorID;
                    }
                    if (meshComponents[idx].GetVertexHandle() != currMesh) {
                        currMesh = meshComponents[idx].GetVertexHandle();
                    }
                    currStartIdx = idx;
                }
            }

            vkCmdEndRenderPass(m_deferredPass.commandBuffer);

            if (vkEndCommandBuffer(m_deferredPass.commandBuffer) != VK_SUCCESS) {
                throw std::runtime_error("failed to record deferred geometry pass command buffer!");
            }

            /// Construct deferred lighting and post processing passes

            // Begin command buffer
            for (uint32_t i = 0; i < m_commandBuffers.size(); ++i) {
                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = nullptr;

                if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }

                // Begin render pass
                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = m_renderPass_deferredLighting;
                if (m_postProcessingPasses.size() > 0) {
                    renderPassInfo.framebuffer = m_framebuffer_deferredLighting;
                } else {
                    renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
                }
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = { m_width, m_height };

                std::array<VkClearValue, 1> clearValues = {};
                clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                JEDeferredShader* deferredShader = (JEDeferredShader*)m_shaderManager.GetShaderAt(materialComponents[0].m_shaderID);
                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, deferredShader->GetPipeline());
                m_shaderManager.GetDescriptorAt(m_deferredLightingDescriptorID).BindDescriptorSets(m_commandBuffers[i], deferredShader->GetPipelineLayout(), 0, i);

                DrawScreenSpaceTriMesh(m_commandBuffers[i]);

                // Draw transluscent geometry using forward rendering
                uint32_t materialIdx = idx;
                if (materialIdx < materialComponents.size()) {
                    currStartIdx = materialIdx;
                    ++materialIdx;
                    currDescriptorID = materialComponents[currStartIdx].m_descriptorID;
                    uint32_t currShaderID = materialComponents[currStartIdx].m_shaderID;
                    JEForwardShader* forwardShader = (JEForwardShader*)m_shaderManager.GetShaderAt(currShaderID);
                    vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, forwardShader->GetPipeline());
                    m_shaderManager.GetDescriptorAt(currDescriptorID).BindDescriptorSets(m_commandBuffers[i], forwardShader->GetPipelineLayout(), 0, i);
                    forwardShader->BindPushConstants_ViewProj(m_commandBuffers[i], camera.GetViewProj());
                    m_shaderManager.GetDescriptorAt(m_forwardModelMatrixDescriptorID).BindDescriptorSets(m_commandBuffers[i], forwardShader->GetPipelineLayout(), 1, i);
                    int currMesh = meshComponents[currStartIdx].GetVertexHandle();
                    
                    while (materialIdx <= materialComponents.size()) {
                        if (materialIdx == materialComponents.size()) {
                            forwardShader->BindPushConstants_InstancedData(m_commandBuffers[i], { currStartIdx, 0, 0, 0 });
                            DrawMeshInstanced(m_commandBuffers[i], materialIdx - currStartIdx, currMesh);
                            break;
                        }
                        if (materialComponents[materialIdx].m_shaderID == currShaderID &&
                            materialComponents[materialIdx].m_descriptorID == currDescriptorID &&
                            meshComponents[materialIdx].GetVertexHandle() == currMesh) {
                            ++materialIdx;
                        } else {
                            // Draw instanced mesh using curr material resources
                            forwardShader->BindPushConstants_InstancedData(m_commandBuffers[i], { currStartIdx, 0, 0, 0 });
                            DrawMeshInstanced(m_commandBuffers[i], materialIdx - currStartIdx, currMesh);

                            if (materialComponents[materialIdx].m_shaderID != currShaderID) {
                                currShaderID = materialComponents[materialIdx].m_shaderID;
                                forwardShader = (JEForwardShader*)m_shaderManager.GetShaderAt(currShaderID);
                                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, forwardShader->GetPipeline());
                                m_shaderManager.GetDescriptorAt(m_forwardModelMatrixDescriptorID).BindDescriptorSets(m_commandBuffers[i], forwardShader->GetPipelineLayout(), 1, i);
                            }
                            if (materialComponents[materialIdx].m_descriptorID != currDescriptorID) {
                                currDescriptorID = materialComponents[materialIdx].m_descriptorID;
                                m_shaderManager.GetDescriptorAt(currDescriptorID).BindDescriptorSets(m_commandBuffers[i], forwardShader->GetPipelineLayout(), 0, i);
                            }
                            if (meshComponents[materialIdx].GetVertexHandle() != currMesh) {
                                currMesh = meshComponents[materialIdx].GetVertexHandle();
                            }
                            currStartIdx = materialIdx;
                        }
                    }
                }

                vkCmdEndRenderPass(m_commandBuffers[i]);

                // Loop over each post processing pass
                for (uint32_t p = 0; p < m_postProcessingPasses.size(); ++p) {
                    VkRenderPassBeginInfo renderPassInfo = {};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    if (p == m_postProcessingPasses.size() - 1) {
                        renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
                        renderPassInfo.renderArea.extent = m_vulkanSwapChain.GetExtent();
                    } else {
                        renderPassInfo.framebuffer = m_postProcessingPasses[p].framebuffer;
                        renderPassInfo.renderArea.extent = { m_width, m_height };
                    }
                    renderPassInfo.renderPass = m_postProcessingPasses[p].renderPass;
                    renderPassInfo.renderArea.offset = { 0, 0 };

                    std::array<VkClearValue, 1> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                    renderPassInfo.pClearValues = clearValues.data();

                    vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_postProcessingShaders[p].GetPipeline());
                    m_postProcessingShaders[p].BindDescriptorSets(m_commandBuffers[i], i);
                    DrawScreenSpaceTriMesh(m_commandBuffers[i]);

                    vkCmdEndRenderPass(m_commandBuffers[i]);
                }

                if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to record command buffer!");
                }
            }
        } else {
        /*
            for (uint32_t i = 0; i < m_commandBuffers.size(); ++i) {
                VkCommandBufferBeginInfo beginInfo = {};
                beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
                beginInfo.pInheritanceInfo = nullptr;

                if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS) {
                    throw std::runtime_error("failed to begin recording command buffer!");
                }

                // Begin render pass
                VkRenderPassBeginInfo renderPassInfo = {};
                renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                renderPassInfo.renderPass = m_forwardPass.renderPass;
                if (m_postProcessingPasses.size() > 0) {
                    renderPassInfo.framebuffer = m_forwardPass.framebuffer;
                } else {
                    renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
                }
                renderPassInfo.renderArea.offset = { 0, 0 };
                renderPassInfo.renderArea.extent = { m_width, m_height };

                std::array<VkClearValue, 2> clearValues = {};
                clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
                clearValues[1].depthStencil = { 1.0f, 0 };
                renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                renderPassInfo.pClearValues = clearValues.data();

                vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                // Draw mesh components
                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_forwardShader.GetPipeline());
                m_forwardShader.BindPushConstants_ViewProj(m_commandBuffers[i], camera.GetViewProj());
                m_forwardShader.BindDescriptorSets(m_commandBuffers[i]);

                for (uint32_t j = 0; j < meshComponents.size(); ++j) {
                    m_forwardShader.BindPushConstants_ModelMatrix(m_commandBuffers[i], transformComponents[j]);
                    DrawMesh(m_commandBuffers[i], meshComponents[j]);
                }

                // Draw bounding boxes / debug geometry
                vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_flatShader.GetPipeline());
                m_flatShader.BindPushConstants_ViewProj(m_commandBuffers[i], camera.GetViewProj());
                vkCmdSetLineWidth(m_commandBuffers[i], 2.0f);

                const std::vector<BoundingBoxData> boundingBoxData = m_meshBufferManager.GetBoundingBoxData();
                for (uint32_t j = 0; j < transformComponents.size(); ++j) {
                    // TODO: move to helper function somehow
                    const glm::vec3& minPos = boundingBoxData[meshComponents[j].GetVertexHandle()][0];
                    const glm::vec3& maxPos = boundingBoxData[meshComponents[j].GetVertexHandle()][7];
                    const glm::vec3 scale = 0.5f * (maxPos - minPos);
                    const glm::mat4 meshTrans = glm::translate(glm::mat4(1.0f), minPos + scale) * glm::scale(glm::mat4(1.0f), scale);
                    m_flatShader.BindPushConstants_ModelMatrix(m_commandBuffers[i], transformComponents[j] * meshTrans);
                    DrawBoundingBoxMesh(m_commandBuffers[i]);
                }

                vkCmdEndRenderPass(m_commandBuffers[i]);

                // Loop over each post processing pass
                for (uint32_t p = 0; p < m_postProcessingPasses.size(); ++p) {
                    VkRenderPassBeginInfo renderPassInfo = {};
                    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                    if (p == m_postProcessingPasses.size() - 1) {
                        renderPassInfo.framebuffer = m_swapChainFramebuffers[i];
                        renderPassInfo.renderArea.extent = m_vulkanSwapChain.GetExtent();
                    } else {
                        renderPassInfo.framebuffer = m_postProcessingPasses[p].framebuffer;
                        renderPassInfo.renderArea.extent = { m_width, m_height };
                    }
                    renderPassInfo.renderPass = m_postProcessingPasses[p].renderPass;
                    renderPassInfo.renderArea.offset = { 0, 0 };

                    std::array<VkClearValue, 1> clearValues = {};
                    clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
                    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
                    renderPassInfo.pClearValues = clearValues.data();

                    vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

                    vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_postProcessingShaders[p].GetPipeline());
                    m_postProcessingShaders[p].BindDescriptorSets(m_commandBuffers[i], i);
                    DrawScreenSpaceTriMesh(m_commandBuffers[i]);

                    vkCmdEndRenderPass(m_commandBuffers[i]);
                }

                if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS) {
                    throw std::runtime_error("failed to record command buffer!");
                }
            }*/
        }
    }

    /// Shadow Pass creation

    void JEVulkanRenderer::CreateShadowRenderPass() {
        VkAttachmentDescription depthAttachment = {};
        depthAttachment.format = FindDepthFormat(m_physicalDevice);
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 0;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.colorAttachmentCount = 0;
        subpass.pColorAttachments = nullptr;
        subpass.pDepthStencilAttachment = &depthAttachmentRef;

        // Use subpass dependencies for layout transitions
        std::array<VkSubpassDependency, 2> dependencies;

        dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[0].dstSubpass = 0;
        dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[0].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        dependencies[1].srcSubpass = 0;
        dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &depthAttachment;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = &subpass;
        renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
        renderPassInfo.pDependencies = dependencies.data();

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_shadowPass.renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void JEVulkanRenderer::CreateShadowFramebuffer() {
        VkImageView depthAttachment = m_shadowPass.depth.imageView;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_shadowPass.renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &depthAttachment;
        framebufferInfo.width = m_shadowPass.width;
        framebufferInfo.height = m_shadowPass.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_shadowPass.framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void JEVulkanRenderer::CreateShadowCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_shadowPass.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate shadow pass command buffer!");
        }

        if (m_shadowPass.semaphore == VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_shadowPass.semaphore) != VK_SUCCESS) {
                throw std::runtime_error("failed to create shadow pass semaphore!");
            }
        }
    }

    void JEVulkanRenderer::CreateShadowPassResources() {
        CreateFramebufferAttachment(m_shadowPass.depth, { m_shadowPass.width, m_shadowPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), FindDepthFormat(m_physicalDevice));
        CreateFramebufferAttachmentSampler(m_shadowPass.depthSampler);
        CreateShadowRenderPass();
        CreateShadowFramebuffer();
    }

    /// Forard Pass creation

    void JEVulkanRenderer::CreateForwardPassRenderPass() {
        std::array<VkAttachmentDescription, 2> attachmentDescs = {};
        for (uint32_t i = 0; i < attachmentDescs.size(); ++i) {
            attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            if (i == 1) {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            } else {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

        attachmentDescs[0].format = VK_FORMAT_R8G8B8A8_UNORM;
        attachmentDescs[1].format = FindDepthFormat(m_physicalDevice);

        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpassDescs = {};
        subpassDescs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescs[0].colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescs[0].pColorAttachments = colorReferences.data();
        subpassDescs[0].pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkSubpassDependency, 2> subpassDependencies;
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = attachmentDescs.size();
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = subpassDescs.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_forwardPass.renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create forward pass render pass!");
        }
    }

    void JEVulkanRenderer::CreateForwardPassFramebuffer() {
        std::array<VkImageView, 2> attachments = { m_forwardPass.color.imageView, m_forwardPass.depth.imageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_forwardPass.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_forwardPass.width;
        framebufferInfo.height = m_forwardPass.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_forwardPass.framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create forward pass framebuffer!");
        }
    }

    void JEVulkanRenderer::CreateForwardPassCommandBuffer() {
        /*VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_forwardPass.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate forward pass command buffer!");
        }*/

        if (m_forwardPass.semaphore == VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_forwardPass.semaphore) != VK_SUCCESS) {
                throw std::runtime_error("failed to create forward pass semaphore!");
            }
        }
    }

    void JEVulkanRenderer::CreateForwardPassResources() {
        CreateFramebufferAttachment(m_forwardPass.color, { m_forwardPass.width, m_forwardPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
        CreateFramebufferAttachment(m_forwardPass.depth, { m_forwardPass.width, m_forwardPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), FindDepthFormat(m_physicalDevice));
        CreateFramebufferAttachmentSampler(m_forwardPass.sampler);
        CreateForwardPassRenderPass();
        CreateForwardPassFramebuffer();
    }

    /// Deferred Rendering Geometry Pass creation

    void JEVulkanRenderer::CreateDeferredPassGeometryRenderPass() {
        std::array<VkAttachmentDescription, 3> attachmentDescs = {};
        for (uint32_t i = 0; i < attachmentDescs.size(); ++i) {
            attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
            attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            if (i == 2) {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
            } else {
                attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
                attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            }
        }

        attachmentDescs[0].format = VK_FORMAT_R8G8B8A8_UNORM; // VK_FORMAT_R16G16B16A16_SFLOAT;
        attachmentDescs[1].format = VK_FORMAT_R8G8B8A8_UNORM; // VK_FORMAT_R16G16B16A16_SFLOAT;
        attachmentDescs[2].format = FindDepthFormat(m_physicalDevice);

        std::vector<VkAttachmentReference> colorReferences;
        colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
        colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

        VkAttachmentReference depthAttachmentRef = {};
        depthAttachmentRef.attachment = 2;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpassDescs = {};
        subpassDescs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescs[0].colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
        subpassDescs[0].pColorAttachments = colorReferences.data();
        subpassDescs[0].pDepthStencilAttachment = &depthAttachmentRef;

        std::array<VkSubpassDependency, 2> subpassDependencies;
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 3;
        renderPassInfo.pAttachments = attachmentDescs.data();
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = subpassDescs.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_deferredPass.renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void JEVulkanRenderer::CreateDeferredPassGeometryFramebuffer() {
        std::array<VkImageView, 3> attachments = { m_deferredPass.color.imageView, m_deferredPass.normal.imageView, m_deferredPass.depth.imageView };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_deferredPass.renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_deferredPass.width;
        framebufferInfo.height = m_deferredPass.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_deferredPass.framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void JEVulkanRenderer::CreateDeferredPassGeometryCommandBuffer() {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = m_commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        if (vkAllocateCommandBuffers(m_device, &allocInfo, &m_deferredPass.commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate deferred pass command buffer!");
        }

        if (m_deferredPass.semaphore == VK_NULL_HANDLE) {
            VkSemaphoreCreateInfo semaphoreInfo = {};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_deferredPass.semaphore) != VK_SUCCESS) {
                throw std::runtime_error("failed to create deferred pass semaphore!");
            }
        }
    }

    void JEVulkanRenderer::CreateDeferredPassGeometryResources() {
        CreateFramebufferAttachment(m_deferredPass.color, { m_deferredPass.width, m_deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM /*VK_FORMAT_R16G16B16A16_SFLOAT*/);
        CreateFramebufferAttachment(m_deferredPass.normal, { m_deferredPass.width, m_deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM /*VK_FORMAT_R16G16B16A16_SFLOAT*/);
        CreateFramebufferAttachment(m_deferredPass.depth, { m_deferredPass.width, m_deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), FindDepthFormat(m_physicalDevice));
        CreateFramebufferAttachmentSampler(m_deferredPass.sampler);
        CreateDeferredPassGeometryRenderPass();
        CreateDeferredPassGeometryFramebuffer();
    }

    /// Deferred Rendering Lighting Pass creation

    void JEVulkanRenderer::CreateDeferredPassLightingRenderPass() {
        VkAttachmentDescription attachmentDesc = {};
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (m_postProcessingPasses.size() > 0) {
            attachmentDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        } else {
            attachmentDesc.format = m_vulkanSwapChain.GetFormat();
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        }

        VkAttachmentReference colorReference;
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpassDescs = {};
        subpassDescs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescs[0].colorAttachmentCount = 1;
        subpassDescs[0].pColorAttachments = &colorReference;
        subpassDescs[0].pDepthStencilAttachment = nullptr;

        std::array<VkSubpassDependency, 2> subpassDependencies;
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDesc;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = subpassDescs.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass_deferredLighting) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void JEVulkanRenderer::CreateDeferredPassLightingFramebuffer() {
        VkImageView attachment = m_framebufferAttachment_deferredLighting.imageView;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass_deferredLighting;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &attachment;
        framebufferInfo.width = m_deferredPass.width;
        framebufferInfo.height = m_deferredPass.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffer_deferredLighting) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void JEVulkanRenderer::CreateDeferredPassLightingResources() {
        CreateDeferredPassLightingRenderPass();
        // If there are no post processing passes, then this pass should render to the screen, meaning no framebuffer should be created
        // This also means that we do not need to create buffers for the framebuffer attachment
        if (m_postProcessingPasses.size() > 0) {
            CreateFramebufferAttachment(m_framebufferAttachment_deferredLighting, { m_width, m_height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
            CreateDeferredPassLightingFramebuffer();
        }
    }

    /// Post Processing passes creation

    void JEVulkanRenderer::CreatePostProcessingPassFramebuffer(uint32_t i) {
        VkImageView attachment;
        if (i == m_postProcessingPasses.size() - 1) {
            return; // This post processing pass will use the swap chain framebuffers, which are already created
        } else {
            attachment = m_postProcessingPasses[i].texture.imageView;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_postProcessingPasses[i].renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &attachment;
        framebufferInfo.width = m_postProcessingPasses[i].width;
        framebufferInfo.height = m_postProcessingPasses[i].height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_postProcessingPasses[i].framebuffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }

    void JEVulkanRenderer::CreatePostProcessingPassRenderPass(uint32_t i) {
        VkAttachmentDescription attachmentDesc = {};
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        if (i == m_postProcessingPasses.size() - 1) {
            attachmentDesc.format = m_vulkanSwapChain.GetFormat();
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        } else {
            attachmentDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
            attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkAttachmentReference colorReference;
        colorReference.attachment = 0;
        colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        std::array<VkSubpassDescription, 1> subpassDescs = {};
        subpassDescs[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpassDescs[0].colorAttachmentCount = 1;
        subpassDescs[0].pColorAttachments = &colorReference;
        subpassDescs[0].pDepthStencilAttachment = nullptr;

        std::array<VkSubpassDependency, 2> subpassDependencies;
        subpassDependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[0].dstSubpass = 0;
        subpassDependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        subpassDependencies[1].srcSubpass = 0;
        subpassDependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
        subpassDependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpassDependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpassDependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpassDependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpassDependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        VkRenderPassCreateInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = 1;
        renderPassInfo.pAttachments = &attachmentDesc;
        renderPassInfo.subpassCount = 1;
        renderPassInfo.pSubpasses = subpassDescs.data();
        renderPassInfo.dependencyCount = static_cast<uint32_t>(subpassDependencies.size());
        renderPassInfo.pDependencies = subpassDependencies.data();

        if (vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_postProcessingPasses[i].renderPass) != VK_SUCCESS) {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void JEVulkanRenderer::CreatePostProcessingPassResources() {
        for (uint32_t i = 0; i < m_postProcessingPasses.size(); ++i) {
            CreateFramebufferAttachmentSampler(m_postProcessingPasses[i].sampler);
            if (i == m_postProcessingPasses.size() - 1) {
                CreateFramebufferAttachment(m_postProcessingPasses[i].texture, { m_postProcessingPasses[i].width, m_postProcessingPasses[i].height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), m_vulkanSwapChain.GetFormat());
            } else {
                CreateFramebufferAttachment(m_postProcessingPasses[i].texture, { m_postProcessingPasses[i].width, m_postProcessingPasses[i].height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
            }
            CreatePostProcessingPassRenderPass(i);
            CreatePostProcessingPassFramebuffer(i);
        }
    }

    void JEVulkanRenderer::SubmitFrame(const std::vector<MaterialComponent>& materialComponents,
        const std::vector<glm::mat4>& transforms, const std::vector<glm::mat4>& transformsSorted) {
        uint32_t imageIndex;
        VkResult result = vkAcquireNextImageKHR(m_device, m_vulkanSwapChain.GetSwapChain(), std::numeric_limits<uint64_t>::max(), m_imageAvailableSemaphores[m_currentFrame], VK_NULL_HANDLE, &imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            RecreateWindowDependentResources();
            return;
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        UpdateShaderBuffers(materialComponents, transforms, transformsSorted, imageIndex);

        // Submit shadow pass command buffer

        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

        VkSubmitInfo submitInfo_shadowPass = {};
        submitInfo_shadowPass.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo_shadowPass.pNext = nullptr;
        submitInfo_shadowPass.pWaitSemaphores = &m_imageAvailableSemaphores[m_currentFrame];
        submitInfo_shadowPass.waitSemaphoreCount = 1;
        submitInfo_shadowPass.pWaitDstStageMask = waitStages;
        submitInfo_shadowPass.pSignalSemaphores = &m_shadowPass.semaphore;
        submitInfo_shadowPass.signalSemaphoreCount = 1;
        submitInfo_shadowPass.commandBufferCount = 1;
        submitInfo_shadowPass.pCommandBuffers = &m_shadowPass.commandBuffer;

        vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(m_graphicsQueue.GetQueue(), 1, &submitInfo_shadowPass, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit shadow pass command buffer!");
        }

        if (m_useDeferred) {
            // Submit deferred render pass with g-buffers
            VkSubmitInfo submitInfo_deferred_gBuffers = {};
            submitInfo_deferred_gBuffers.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo_deferred_gBuffers.pNext = nullptr;
            submitInfo_deferred_gBuffers.waitSemaphoreCount = 1;
            submitInfo_deferred_gBuffers.pWaitSemaphores = &m_shadowPass.semaphore;
            submitInfo_deferred_gBuffers.pWaitDstStageMask = waitStages;
            submitInfo_deferred_gBuffers.commandBufferCount = 1;
            submitInfo_deferred_gBuffers.pCommandBuffers = &m_deferredPass.commandBuffer;
            submitInfo_deferred_gBuffers.signalSemaphoreCount = 1;
            submitInfo_deferred_gBuffers.pSignalSemaphores = &m_deferredPass.semaphore;

            vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
            vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

            if (vkQueueSubmit(m_graphicsQueue.GetQueue(), 1, &submitInfo_deferred_gBuffers, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
                throw std::runtime_error("failed to submit deferred geometry command buffer!");
            }
        }

        // Submit render-to-screen command buffer

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.pNext = nullptr;
        submitInfo.waitSemaphoreCount = 1;

        if (m_useDeferred) {
            submitInfo.pWaitSemaphores = &m_deferredPass.semaphore;
        } else {
            submitInfo.pWaitSemaphores = &m_shadowPass.semaphore;
        }

        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_commandBuffers[imageIndex];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

        vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
        vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

        if (vkQueueSubmit(m_graphicsQueue.GetQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS) {
            throw std::runtime_error("failed to submit command buffer!");
        }
        
        // TODO: this limits # of frames in flight to 1
        vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

        VkPresentInfoKHR presentInfo = {};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &m_renderFinishedSemaphores[m_currentFrame];

        VkSwapchainKHR swapChains[] = { m_vulkanSwapChain.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        result = vkQueuePresentKHR(m_presentationQueue.GetQueue(), &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_didFramebufferResize) {
            m_didFramebufferResize = false;
            RecreateWindowDependentResources();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        m_currentFrame = (m_currentFrame + 1) % m_MAX_FRAMES_IN_FLIGHT;
    }

    void JEVulkanRenderer::CleanupWindowDependentResources() {
        // Swap Chain Framebuffers
        for (auto framebuffer : m_swapChainFramebuffers) {
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
        }

        // Deferred Pass - Geometry
        vkDestroyImage(m_device, m_deferredPass.color.image, nullptr);
        vkDestroyImage(m_device, m_deferredPass.normal.image, nullptr);
        vkDestroyImage(m_device, m_deferredPass.depth.image, nullptr);
        vkFreeMemory(m_device, m_deferredPass.color.deviceMemory, nullptr);
        vkFreeMemory(m_device, m_deferredPass.normal.deviceMemory, nullptr);
        vkFreeMemory(m_device, m_deferredPass.depth.deviceMemory, nullptr);
        vkDestroyImageView(m_device, m_deferredPass.color.imageView, nullptr);
        vkDestroyImageView(m_device, m_deferredPass.normal.imageView, nullptr);
        vkDestroyImageView(m_device, m_deferredPass.depth.imageView, nullptr);
        vkDestroyRenderPass(m_device, m_deferredPass.renderPass, nullptr);
        vkDestroyFramebuffer(m_device, m_deferredPass.framebuffer, nullptr);
        vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_deferredPass.commandBuffer);
        vkDestroySampler(m_device, m_deferredPass.sampler, nullptr);

        // Deferred Pass - Lighting
        vkDestroyRenderPass(m_device, m_renderPass_deferredLighting, nullptr);
        if (m_postProcessingPasses.size() > 0) {
            vkDestroyImage(m_device, m_framebufferAttachment_deferredLighting.image, nullptr);
            vkFreeMemory(m_device, m_framebufferAttachment_deferredLighting.deviceMemory, nullptr);
            vkDestroyImageView(m_device, m_framebufferAttachment_deferredLighting.imageView, nullptr);
            vkDestroyFramebuffer(m_device, m_framebuffer_deferredLighting, nullptr);
        }

        // Forward Pass
        vkDestroyImage(m_device, m_forwardPass.color.image, nullptr);
        vkDestroyImage(m_device, m_forwardPass.depth.image, nullptr);
        vkFreeMemory(m_device, m_forwardPass.color.deviceMemory, nullptr);
        vkFreeMemory(m_device, m_forwardPass.depth.deviceMemory, nullptr);
        vkDestroyImageView(m_device, m_forwardPass.color.imageView, nullptr);
        vkDestroyImageView(m_device, m_forwardPass.depth.imageView, nullptr);
        vkDestroyRenderPass(m_device, m_forwardPass.renderPass, nullptr);
        vkDestroyFramebuffer(m_device, m_forwardPass.framebuffer, nullptr);
        vkDestroySampler(m_device, m_forwardPass.sampler, nullptr);

        // Post Processing
        for (uint32_t p = 0; p < m_postProcessingPasses.size(); ++p) {
            vkDestroyImage(m_device, m_postProcessingPasses[p].texture.image, nullptr);
            vkFreeMemory(m_device, m_postProcessingPasses[p].texture.deviceMemory, nullptr);
            vkDestroyImageView(m_device, m_postProcessingPasses[p].texture.imageView, nullptr);
            vkDestroyRenderPass(m_device, m_postProcessingPasses[p].renderPass, nullptr);
            if (m_postProcessingPasses[p].framebuffer != VK_NULL_HANDLE) {
                vkDestroyFramebuffer(m_device, m_postProcessingPasses[p].framebuffer, nullptr);
            }
            vkDestroySampler(m_device, m_postProcessingPasses[p].sampler, nullptr);
        }

        vkFreeCommandBuffers(m_device, m_commandPool, static_cast<uint32_t>(m_commandBuffers.size()), m_commandBuffers.data());
        CleanupShaders();
        m_vulkanSwapChain.Cleanup(m_device);
    }

    void JEVulkanRenderer::RecreateWindowDependentResources() {
        int newWidth = 0, newHeight = 0;
        while (newWidth == 0 || newHeight == 0) {
            m_vulkanWindow.AwaitMaximize(&newWidth, &newHeight);
        }
        m_width = newWidth;
        m_height = newHeight;

        vkDeviceWaitIdle(m_device);

        CleanupWindowDependentResources();
        m_vulkanSwapChain.Create(m_physicalDevice, m_device, m_vulkanWindow, newWidth, newHeight);

        // Forward Pass
        m_forwardPass.width = newWidth;
        m_forwardPass.height = newHeight;
        CreateForwardPassResources();

        // Deferred Pass - Geometry
        m_deferredPass.width = newWidth;
        m_deferredPass.height = newHeight;
        CreateDeferredPassGeometryResources();

        // Post Processing
        m_postProcessingPasses.clear();
        /*JEPostProcessingPass p;
        p.width = newWidth;
        p.height = newHeight;
        p.shaderIndex = 0;
        m_postProcessingPasses.push_back(p);
        p.shaderIndex = 1;
        m_postProcessingPasses.push_back(p);*/

        // Deferred Pass - Lighting
        CreateDeferredPassLightingRenderPass();
        if (m_postProcessingPasses.size() > 0) {
            CreateFramebufferAttachment(m_framebufferAttachment_deferredLighting, { m_width, m_height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
            CreateDeferredPassLightingFramebuffer();
        }

        CreatePostProcessingPassResources();
        CreateSwapChainFramebuffers();

        m_sceneManager->RecreateResources({ m_width, m_height });
        // TODO: recreate shaders
        CreateShadowCommandBuffer();
        CreateDeferredPassGeometryCommandBuffer();
        CreateDeferredLightingAndPostProcessingCommandBuffer();
    }

    void JEVulkanRenderer::RegisterCallbacks(JEIOHandler* ioHandler) {
        /*JECallbackFunction loadScene0 = [&] {
            vkDeviceWaitIdle(m_device);
            m_meshBufferManager.Cleanup();
            CleanupTextures();
            CleanupShaders();
            m_sceneManager->LoadScene(0, { m_width, m_height }, { m_shadowPass.width, m_shadowPass.height });
            CreateShadowCommandBuffer();
            CreateDeferredPassGeometryCommandBuffer();
            CreateDeferredLightingAndPostProcessingCommandBuffer();
        };
        JECallbackFunction loadScene1 = [&] {
            vkDeviceWaitIdle(m_device);
            m_meshBufferManager.Cleanup();
            CleanupTextures();
            CleanupShaders();
            m_sceneManager->LoadScene(1, { m_width, m_height }, { m_shadowPass.width, m_shadowPass.height });
            CreateShadowCommandBuffer();
            CreateDeferredPassGeometryCommandBuffer();
            CreateDeferredLightingAndPostProcessingCommandBuffer();
        };
        JECallbackFunction loadScene2 = [&] {
            vkDeviceWaitIdle(m_device);
            m_meshBufferManager.Cleanup();
            CleanupTextures();
            CleanupShaders();
            m_sceneManager->LoadScene(2, { m_width, m_height }, { m_shadowPass.width, m_shadowPass.height });
            CreateShadowCommandBuffer();
            CreateDeferredPassGeometryCommandBuffer();
            CreateDeferredLightingAndPostProcessingCommandBuffer();
        };
        ioHandler->AddCallback(JE_KEY_0, loadScene0);
        ioHandler->AddCallback(JE_KEY_1, loadScene1);
        ioHandler->AddCallback(JE_KEY_2, loadScene2);*/
    }
}
