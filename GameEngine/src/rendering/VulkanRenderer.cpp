#include <map>
#include <vector>

#include "VulkanRenderer.h"
#include "../scene/SceneManager.h"
#include "../EngineApplication.h"

static void JEFramebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto renderer = reinterpret_cast<JEEngineApplication*>(glfwGetWindowUserPointer(window))->GetRenderSubsystem();
    renderer->FramebufferResized();
}

void JEVulkanRenderer::Initialize(JESceneManager* sceneManager) {
    // Window (GLFW)
    vulkanWindow.Initialize(width, height, "VulkanWindow", instance);

    /// Vulkan setup

    // Vulkan Instance
    CreateVulkanInstance();

    // Validation Layers
    vulkanValidationLayers.SetupDebugCallback(instance);

    // Window surface
    vulkanWindow.SetupVulkanSurface(instance);
    vulkanWindow.SetFrameBufferCallback(JEFramebufferResizeCallback);

    // Devices
    PickPhysicalDevice();
    CreateLogicalDevice();

    // Swap Chain
    vulkanSwapChain.Create(physicalDevice, device, vulkanWindow, width, height);

    // Command Pool
    CreateCommandPool();

    // Create deferred lighting pass framebuffer attachments
    //CreateDepthAttachment(depthBuffer, { width, height }, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

    // Add the post processing passes
    JEPostProcessingPass p;
    p.shaderIndex = 0;
    postProcessingPasses.push_back(p);
    p.shaderIndex = 1;
    postProcessingPasses.push_back(p);

    // Create the post processing pass(es)
    CreatePostProcessingPassResources();
    
    // Shadow Pass(es)
    CreateShadowPassResources();
    
    // Deferred Rendering Passes
    CreateDeferredPassGeometryResources();
    CreateDeferredPassLightingResources();

    // Create the swap chain framebuffers
    CreateSwapChainFramebuffers();

    // Load Scene
    this->sceneManager = sceneManager;
    this->sceneManager->LoadScene(physicalDevice, device, commandPool, renderPass_deferredLighting, framebufferAttachment_deferredLighting.imageView, graphicsQueue, vulkanSwapChain, shadowPass, deferredPass, postProcessingPasses, 0);

    // Command buffers
    CreateShadowCommandBuffer();
    CreateDeferredPassGeometryCommandBuffer();
    CreateDeferredLightingAndPostProcessingCommandBuffer();

    // Sync objects
    CreateSemaphoresAndFences();
}

void JEVulkanRenderer::Cleanup() {
    CleanupWindowDependentRenderingResources();
    sceneManager->CleanupMeshesAndTextures(device);
    
    // Cleanup shadow pass
    vkDestroyImage(device, shadowPass.depth.image, nullptr);
    vkFreeMemory(device, shadowPass.depth.deviceMemory, nullptr);
    vkDestroyImageView(device, shadowPass.depth.imageView, nullptr);
    vkDestroySampler(device, shadowPass.depthSampler, nullptr);
    vkDestroyRenderPass(device, shadowPass.renderPass, nullptr);
    vkDestroyFramebuffer(device, shadowPass.framebuffer, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &shadowPass.commandBuffer);
    vkDestroySemaphore(device, shadowPass.semaphore, nullptr);
    
    // Deferred Pass - Geometry
    vkDestroySemaphore(device, deferredPass.semaphore, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(device, inFlightFences[i], nullptr);
    }

    vkDestroyCommandPool(device, commandPool, nullptr);

    vkDestroyDevice(device, nullptr);
    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
        vulkanValidationLayers.DestroyDebugCallback(instance);
    }
    vulkanWindow.Cleanup(instance);
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char*> JEVulkanRenderer::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = vulkanWindow.GetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
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

    // Application can't function without geometry shaders
    // TODO long term: change device requirements as engine develops
    /*if (!deviceFeatures.geometryShader) {
    return 0;
    }*/

    bool extensionsSupported = vulkanSwapChain.CheckDeviceExtensionSupport(physicalDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = vulkanSwapChain.QuerySwapChainSupport(physicalDevice, vulkanWindow.GetSurface());
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
    if (vulkanValidationLayers.AreValidationLayersEnabled() && !vulkanValidationLayers.CheckValidationLayerSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Game Engine App";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Joe Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto extensions = GetRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
        const std::vector<const char*>& validationLayers = vulkanValidationLayers.GetValidationLayers();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }
}

void JEVulkanRenderer::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // Use an ordered map to automatically sort candidates by increasing score
    std::multimap<int, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        int score = RateDeviceSuitability(device, vulkanWindow);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }
}

void JEVulkanRenderer::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, vulkanWindow.GetSurface());

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = GetQueueCreateInfos(indices);
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    deviceFeatures.samplerAnisotropy = VK_TRUE;
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> deviceExtensions = JEVulkanSwapChain::GetDeviceExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
        const std::vector<const char*>& validationLayers = vulkanValidationLayers.GetValidationLayers();
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device!");
    }

    graphicsQueue.GetDeviceQueue(device, indices.graphicsFamily.value());
    presentationQueue.GetDeviceQueue(device, indices.presentFamily.value());
}

void JEVulkanRenderer::CreateSwapChainFramebuffers() {
    const std::vector<VkImageView>& swapChainImageViews = vulkanSwapChain.GetImageViews();
    swapChainFramebuffers.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); ++i) {
        VkImageView attachment = swapChainImageViews[i];
        VkExtent2D extent = vulkanSwapChain.GetExtent();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = &attachment;
        framebufferInfo.width = extent.width;
        framebufferInfo.height = extent.height;
        framebufferInfo.layers = 1;

        if (postProcessingPasses.size() > 0) {
            framebufferInfo.renderPass = postProcessingPasses[postProcessingPasses.size() - 1].renderPass;
        } else {
            framebufferInfo.renderPass = renderPass_deferredLighting;
        }

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void JEVulkanRenderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(physicalDevice, vulkanWindow.GetSurface());

    VkCommandPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    poolInfo.flags = 0;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool!");
    }
}

void JEVulkanRenderer::CreateDeferredLightingAndPostProcessingCommandBuffer() {
    commandBuffers.resize(swapChainFramebuffers.size());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }

    // Command Buffer recording

    // Begin command buffer
    for (size_t i = 0; i < commandBuffers.size(); ++i) {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(commandBuffers[i], &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        // Begin render pass
        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass_deferredLighting;
        renderPassInfo.framebuffer = framebuffer_deferredLighting;
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = { width, height };

        std::array<VkClearValue, 1> clearValues = {};
        clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        sceneManager->BindDeferredPassLightingResources(commandBuffers[i], i);

        vkCmdEndRenderPass(commandBuffers[i]);

        // Loop over each post processing pass
        for (uint32_t p = 0; p < postProcessingPasses.size(); ++p) {
            VkRenderPassBeginInfo renderPassInfo = {};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            if (p == postProcessingPasses.size() - 1) {
                renderPassInfo.framebuffer = swapChainFramebuffers[i];
                renderPassInfo.renderArea.extent = vulkanSwapChain.GetExtent();
            } else {
                renderPassInfo.framebuffer = postProcessingPasses[p].framebuffer;
                renderPassInfo.renderArea.extent = { width, height };
            }
            renderPassInfo.renderPass = postProcessingPasses[p].renderPass;
            renderPassInfo.renderArea.offset = { 0, 0 };

            std::array<VkClearValue, 1> clearValues = {};
            clearValues[0].color = { 0.0f, 0.0f, 0.0f, 1.0f };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            sceneManager->BindPostProcessingPassResources(commandBuffers[i], i, p);

            vkCmdEndRenderPass(commandBuffers[i]);
        }

        if (vkEndCommandBuffer(commandBuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to record command buffer!");
        }
    }
}

void JEVulkanRenderer::CreateSemaphoresAndFences() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
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
    CreateImage(physicalDevice, device, extent.width, extent.height, format, VK_IMAGE_TILING_OPTIMAL, usageBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, attachment.image, attachment.deviceMemory);
    attachment.imageView = CreateImageView(device, attachment.image, format, aspectMask);
    TransitionImageLayout(device, commandPool, graphicsQueue, attachment.image, format, VK_IMAGE_LAYOUT_UNDEFINED, imageLayout);
}

void JEVulkanRenderer::CreateFramebufferAttachmentSampler(VkSampler& sampler) {
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

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

    if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture sampler!");
    }
}

/// Shadow Pass creation

void JEVulkanRenderer::CreateShadowRenderPass() {
    VkAttachmentDescription depthAttachment = {};
    depthAttachment.format = FindDepthFormat(physicalDevice);
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef = {};
    depthAttachmentRef.attachment = 0;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 0;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    // Use subpass dependencies for layout transitions
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    
    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &shadowPass.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void JEVulkanRenderer::CreateShadowFramebuffer() {
    VkImageView depthAttachment = shadowPass.depth.imageView;

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = shadowPass.renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &depthAttachment;
    framebufferInfo.width = shadowPass.width;
    framebufferInfo.height = shadowPass.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &shadowPass.framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void JEVulkanRenderer::CreateShadowCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &shadowPass.commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate shadow pass command buffer!");
    }

    if (shadowPass.semaphore == VK_NULL_HANDLE) {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &shadowPass.semaphore) != VK_SUCCESS) {
            throw std::runtime_error("failed to create shadow pass semaphore!");
        }
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(shadowPass.commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Begin render pass

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = shadowPass.renderPass;
    renderPassInfo.framebuffer = shadowPass.framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { shadowPass.width, shadowPass.height };

    VkClearValue clearValue = { 1.0f, 0 };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearValue;

    vkCmdBeginRenderPass(shadowPass.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    sceneManager->BindShadowPassResources(shadowPass.commandBuffer);

    vkCmdEndRenderPass(shadowPass.commandBuffer);

    if (vkEndCommandBuffer(shadowPass.commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void JEVulkanRenderer::CreateShadowPassResources() {
    CreateFramebufferAttachment(shadowPass.depth, { shadowPass.width, shadowPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), FindDepthFormat(physicalDevice));
    CreateFramebufferAttachmentSampler(shadowPass.depthSampler);
    CreateShadowRenderPass();
    CreateShadowFramebuffer();
}

/// Deferred Rendering Geometry Pass creation

void JEVulkanRenderer::CreateDeferredPassGeometryRenderPass() {
    std::array<VkAttachmentDescription, 3> attachmentDescs = {};
    for (uint32_t i = 0; i < 3; ++i) {
        attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        if (i == 2) {
            attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        } else {
            attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
    }

    attachmentDescs[0].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachmentDescs[1].format = VK_FORMAT_R16G16B16A16_SFLOAT;
    attachmentDescs[2].format = FindDepthFormat(physicalDevice);

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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &deferredPass.renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void JEVulkanRenderer::CreateDeferredPassGeometryFramebuffer() {
    std::array<VkImageView, 3> attachments = { deferredPass.color.imageView, deferredPass.normal.imageView, deferredPass.depth.imageView };

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = deferredPass.renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = deferredPass.width;
    framebufferInfo.height = deferredPass.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &deferredPass.framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void JEVulkanRenderer::CreateDeferredPassGeometryCommandBuffer() {
    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(device, &allocInfo, &deferredPass.commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate deferred pass command buffer!");
    }

    if (deferredPass.semaphore == VK_NULL_HANDLE) {
        VkSemaphoreCreateInfo semaphoreInfo = {};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &deferredPass.semaphore) != VK_SUCCESS) {
            throw std::runtime_error("failed to create deferred pass semaphore!");
        }
    }

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    if (vkBeginCommandBuffer(deferredPass.commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // Begin render pass

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = deferredPass.renderPass;
    renderPassInfo.framebuffer = deferredPass.framebuffer;
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = { deferredPass.width, deferredPass.height };

    std::array<VkClearValue, 3> clearValues;
    clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
    clearValues[2].depthStencil = { 1.0f, 0 };
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(deferredPass.commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    sceneManager->BindDeferredPassGeometryResources(deferredPass.commandBuffer);

    vkCmdEndRenderPass(deferredPass.commandBuffer);

    if (vkEndCommandBuffer(deferredPass.commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void JEVulkanRenderer::CreateDeferredPassGeometryResources() {
    CreateFramebufferAttachment(deferredPass.color, { deferredPass.width, deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R16G16B16A16_SFLOAT);
    CreateFramebufferAttachment(deferredPass.normal, { deferredPass.width, deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R16G16B16A16_SFLOAT);
    CreateFramebufferAttachment(deferredPass.depth, { deferredPass.width, deferredPass.height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), FindDepthFormat(physicalDevice));
    CreateFramebufferAttachmentSampler(deferredPass.sampler);
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
    attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    attachmentDesc.format = VK_FORMAT_R8G8B8A8_UNORM;

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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass_deferredLighting) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void JEVulkanRenderer::CreateDeferredPassLightingFramebuffer() {
    VkImageView attachment = framebufferAttachment_deferredLighting.imageView;

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass_deferredLighting;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;
    framebufferInfo.width = deferredPass.width;
    framebufferInfo.height = deferredPass.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffer_deferredLighting) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void JEVulkanRenderer::CreateDeferredPassLightingResources() {
    CreateDeferredPassLightingRenderPass();
    // If there are no post processing passes, then this pass should render to the screen, meaning no framebuffer should be created
    // This also means that we do not need to create buffers for the framebuffer attachment
    if (postProcessingPasses.size() > 0) {
        CreateFramebufferAttachment(framebufferAttachment_deferredLighting, { width, height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
        CreateDeferredPassLightingFramebuffer();
    }
}

/// Post Processing passes creation

void JEVulkanRenderer::CreatePostProcessingPassFramebuffer(uint32_t i) {
    VkImageView attachment;
    if (i == postProcessingPasses.size() - 1) {
        return; // This post processing pass will use the swap chain framebuffers, which are already created
    } else {
        attachment = postProcessingPasses[i].texture.imageView;
    }

    VkFramebufferCreateInfo framebufferInfo = {};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = postProcessingPasses[i].renderPass;
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.pAttachments = &attachment;
    framebufferInfo.width = postProcessingPasses[i].width;
    framebufferInfo.height = postProcessingPasses[i].height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &postProcessingPasses[i].framebuffer) != VK_SUCCESS) {
        throw std::runtime_error("failed to create framebuffer!");
    }
}

void JEVulkanRenderer::CreatePostProcessingPassRenderPass(uint32_t i) {
    VkAttachmentDescription attachmentDesc = {};
    if (i == postProcessingPasses.size() - 1) {
        attachmentDesc.format = vulkanSwapChain.GetFormat();
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    } else {
        attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
        attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        attachmentDesc.format = VK_FORMAT_R8G8B8A8_UNORM;
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

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &postProcessingPasses[i].renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void JEVulkanRenderer::CreatePostProcessingPassResources() {
    for (uint32_t i = 0; i < postProcessingPasses.size(); ++i) {
        CreateFramebufferAttachmentSampler(postProcessingPasses[i].sampler);
        if (i == postProcessingPasses.size() - 1) {
            CreateFramebufferAttachment(postProcessingPasses[i].texture, { postProcessingPasses[i].width, postProcessingPasses[i].height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), vulkanSwapChain.GetFormat());
        } else {
            CreateFramebufferAttachment(postProcessingPasses[i].texture, { postProcessingPasses[i].width, postProcessingPasses[i].height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
        }
        CreatePostProcessingPassRenderPass(i);
        CreatePostProcessingPassFramebuffer(i);
    }
}

void JEVulkanRenderer::DrawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, vulkanSwapChain.GetSwapChain(), std::numeric_limits<uint64_t>::max(), imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateWindowDependentRenderingResources();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    sceneManager->UpdateModelMatrices();
    sceneManager->UpdateShaderUniformBuffers(device, imageIndex);

    // Submit shadow pass command buffer

    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    VkSubmitInfo submitInfo_shadowPass = {};
    submitInfo_shadowPass.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo_shadowPass.pWaitSemaphores = &imageAvailableSemaphores[currentFrame];
    submitInfo_shadowPass.waitSemaphoreCount = 1;
    submitInfo_shadowPass.pWaitDstStageMask = waitStages;
    submitInfo_shadowPass.pSignalSemaphores = &shadowPass.semaphore;
    submitInfo_shadowPass.signalSemaphoreCount = 1;
    submitInfo_shadowPass.commandBufferCount = 1;
    submitInfo_shadowPass.pCommandBuffers = &shadowPass.commandBuffer;

    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue.GetQueue(), 1, &submitInfo_shadowPass, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }
    
    // Submit deferred render pass with g-buffers

    VkSubmitInfo submitInfo_deferred_gBuffers = {};
    submitInfo_deferred_gBuffers.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo_deferred_gBuffers.waitSemaphoreCount = 1;
    submitInfo_deferred_gBuffers.pWaitSemaphores = &shadowPass.semaphore;
    submitInfo_deferred_gBuffers.pWaitDstStageMask = waitStages;
    submitInfo_deferred_gBuffers.commandBufferCount = 1;
    submitInfo_deferred_gBuffers.pCommandBuffers = &deferredPass.commandBuffer;
    submitInfo_deferred_gBuffers.signalSemaphoreCount = 1;
    submitInfo_deferred_gBuffers.pSignalSemaphores = &deferredPass.semaphore;

    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue.GetQueue(), 1, &submitInfo_deferred_gBuffers, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    // Submit render-to-screen command buffer

    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &deferredPass.semaphore;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &renderFinishedSemaphores[currentFrame];

    vkWaitForFences(device, 1, &inFlightFences[currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
    vkResetFences(device, 1, &inFlightFences[currentFrame]);

    if (vkQueueSubmit(graphicsQueue.GetQueue(), 1, &submitInfo, inFlightFences[currentFrame]) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &renderFinishedSemaphores[currentFrame];

    VkSwapchainKHR swapChains[] = { vulkanSwapChain.GetSwapChain() };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pResults = nullptr;

    result = vkQueuePresentKHR(presentationQueue.GetQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || framebufferResized) {
        framebufferResized = false;
        RecreateWindowDependentRenderingResources();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void JEVulkanRenderer::CleanupWindowDependentRenderingResources() {
    // Swap Chain Framebuffers
    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }

    // Deferred Pass - Geometry
    vkDestroyImage(device, deferredPass.color.image, nullptr);
    vkDestroyImage(device, deferredPass.normal.image, nullptr);
    vkDestroyImage(device, deferredPass.depth.image, nullptr);
    vkFreeMemory(device, deferredPass.color.deviceMemory, nullptr);
    vkFreeMemory(device, deferredPass.normal.deviceMemory, nullptr);
    vkFreeMemory(device, deferredPass.depth.deviceMemory, nullptr);
    vkDestroyImageView(device, deferredPass.color.imageView, nullptr);
    vkDestroyImageView(device, deferredPass.normal.imageView, nullptr);
    vkDestroyImageView(device, deferredPass.depth.imageView, nullptr);
    vkDestroyRenderPass(device, deferredPass.renderPass, nullptr);
    vkDestroyFramebuffer(device, deferredPass.framebuffer, nullptr);
    vkFreeCommandBuffers(device, commandPool, 1, &deferredPass.commandBuffer);
    vkDestroySampler(device, deferredPass.sampler, nullptr);
    
    // Deferred Pass - Lighting
    vkDestroyRenderPass(device, renderPass_deferredLighting, nullptr);
    if (postProcessingPasses.size() > 0) {
        vkDestroyImage(device, framebufferAttachment_deferredLighting.image, nullptr);
        vkFreeMemory(device, framebufferAttachment_deferredLighting.deviceMemory, nullptr);
        vkDestroyImageView(device, framebufferAttachment_deferredLighting.imageView, nullptr);
        vkDestroyFramebuffer(device, framebuffer_deferredLighting, nullptr);
    }

    // Post Processing
    for (uint32_t p = 0; p < postProcessingPasses.size(); ++p) {
        vkDestroyImage(device, postProcessingPasses[p].texture.image, nullptr);
        vkFreeMemory(device, postProcessingPasses[p].texture.deviceMemory, nullptr);
        vkDestroyImageView(device, postProcessingPasses[p].texture.imageView, nullptr);
        vkDestroyRenderPass(device, postProcessingPasses[p].renderPass, nullptr);
        if (postProcessingPasses[p].framebuffer != VK_NULL_HANDLE) {
            vkDestroyFramebuffer(device, postProcessingPasses[p].framebuffer, nullptr);
        }
        vkDestroySampler(device, postProcessingPasses[p].sampler, nullptr);
    }

    vkFreeCommandBuffers(device, commandPool, static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
    sceneManager->CleanupShaders(device);
    vulkanSwapChain.Cleanup(device);
}

void JEVulkanRenderer::RecreateWindowDependentRenderingResources() {
    int newWidth = 0, newHeight = 0;
    while (newWidth == 0 || newHeight == 0) {
        vulkanWindow.AwaitMaximize(&newWidth, &newHeight);
    }
    width = newWidth;
    height = newHeight;

    vkDeviceWaitIdle(device);

    CleanupWindowDependentRenderingResources();
    vulkanSwapChain.Create(physicalDevice, device, vulkanWindow, newWidth, newHeight);

    // Deferred Pass - Geometry
    deferredPass.width = newWidth;
    deferredPass.height = newHeight;
    CreateDeferredPassGeometryResources();

    // Post Processing
    postProcessingPasses.clear();
    JEPostProcessingPass p;
    p.width = newWidth;
    p.height = newHeight;
    p.shaderIndex = 0;
    postProcessingPasses.push_back(p);
    p.shaderIndex = 1;
    postProcessingPasses.push_back(p);

    // Deferred Pass - Lighting
    CreateDeferredPassLightingRenderPass();
    if (postProcessingPasses.size() > 0) {
        CreateFramebufferAttachment(framebufferAttachment_deferredLighting, { width, height }, static_cast<VkImageUsageFlagBits>(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT), VK_FORMAT_R8G8B8A8_UNORM);
        CreateDeferredPassLightingFramebuffer();
    }

    CreatePostProcessingPassResources();
    CreateSwapChainFramebuffers();

    sceneManager->RecreateResources(physicalDevice, device, vulkanSwapChain, renderPass_deferredLighting, framebufferAttachment_deferredLighting.imageView, shadowPass, deferredPass, postProcessingPasses);
    CreateShadowCommandBuffer();
    CreateDeferredPassGeometryCommandBuffer();
    CreateDeferredLightingAndPostProcessingCommandBuffer();
}

void JEVulkanRenderer::RegisterCallbacks(JEIOHandler* ioHandler) {
    JECallbackFunction loadScene0 = [&] {
        vkDeviceWaitIdle(device);
        sceneManager->CleanupMeshesAndTextures(device);
        sceneManager->CleanupShaders(device);
        sceneManager->LoadScene(physicalDevice, device, commandPool, renderPass_deferredLighting, framebufferAttachment_deferredLighting.imageView, graphicsQueue, vulkanSwapChain, shadowPass, deferredPass, postProcessingPasses, 0);
        CreateShadowCommandBuffer();
        CreateDeferredPassGeometryCommandBuffer();
        CreateDeferredLightingAndPostProcessingCommandBuffer();
    };
    JECallbackFunction loadScene1 = [&] {
        vkDeviceWaitIdle(device);
        sceneManager->CleanupMeshesAndTextures(device);
        sceneManager->CleanupShaders(device);
        sceneManager->LoadScene(physicalDevice, device, commandPool, renderPass_deferredLighting, framebufferAttachment_deferredLighting.imageView, graphicsQueue, vulkanSwapChain, shadowPass, deferredPass, postProcessingPasses, 1);
        CreateShadowCommandBuffer();
        CreateDeferredPassGeometryCommandBuffer();
        CreateDeferredLightingAndPostProcessingCommandBuffer();
    };
    JECallbackFunction loadScene2 = [&] {
        vkDeviceWaitIdle(device);
        sceneManager->CleanupMeshesAndTextures(device);
        sceneManager->CleanupShaders(device);
        sceneManager->LoadScene(physicalDevice, device, commandPool, renderPass_deferredLighting, framebufferAttachment_deferredLighting.imageView, graphicsQueue, vulkanSwapChain, shadowPass, deferredPass, postProcessingPasses, 2);
        CreateShadowCommandBuffer();
        CreateDeferredPassGeometryCommandBuffer();
        CreateDeferredLightingAndPostProcessingCommandBuffer();
    };
    ioHandler->AddCallback(JE_KEY_0, loadScene0);
    ioHandler->AddCallback(JE_KEY_1, loadScene1);
    ioHandler->AddCallback(JE_KEY_2, loadScene2);
}
