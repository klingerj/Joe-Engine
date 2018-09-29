#include <map>
#include <vector>

#include "VulkanRenderer.h"

void VulkanRenderer::Initialize() {
    width = DEFAULT_SCREEN_WIDTH;
    height = DEFAULT_SCREEN_HEIGHT;

    // Window (GLFW)
    vulkanWindow.Initialize(width, height, "VulkanWindow", instance);

    /// Vulkan setup

    // Vulkan Instance
    CreateVulkanInstance();

    // Validation Layers
    vulkanValidationLayers.SetupDebugCallback(instance);

    // Window surface
    vulkanWindow.SetupVulkanSurface(instance);

    // Devices
    PickPhysicalDevice();
    CreateLogicalDevice();

    // Swap Chain
    vulkanSwapChain.Create(physicalDevice, device, vulkanWindow.GetSurface(), width, height);

    // Render pass(es)
    CreateRenderPass(vulkanSwapChain);

    // Shaders
    shaders[0] = VulkanShader(device, vulkanSwapChain, renderPass,
        SHADER_DIR + "vert_basic.spv", SHADER_DIR + "frag_basic.spv");
}

void VulkanRenderer::Cleanup() {
    for (int i = 0; i < NUM_VULKAN_SHADERS; ++i) {
        shaders[i].Cleanup(device);
    }
    vkDestroyRenderPass(device, renderPass, nullptr);
    vulkanSwapChain.Cleanup(device);
    vkDestroyDevice(device, nullptr);
    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
        vulkanValidationLayers.DestroyDebugCallback(instance);
    }
    vulkanWindow.Cleanup(instance);
    vkDestroyInstance(instance, nullptr);
}

std::vector<const char*> VulkanRenderer::GetRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = vulkanWindow.GetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (vulkanValidationLayers.AreValidationLayersEnabled()) {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    return extensions;
}

int VulkanRenderer::RateDeviceSuitability(const VkPhysicalDevice& physDevice, const VulkanWindow& vulkanWindow) {
    VkPhysicalDeviceProperties deviceProperties;
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceProperties(physDevice, &deviceProperties);
    vkGetPhysicalDeviceFeatures(physDevice, &deviceFeatures);

    int score = 0;

    QueueFamilyIndices indices = FindQueueFamilies(physDevice, vulkanWindow.GetSurface());
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
    // TODO remove this
    // TODO long term: change device requirements as engine develops
    /*if (!deviceFeatures.geometryShader) {
    return 0;
    }*/

    bool extensionsSupported = vulkanSwapChain.CheckDeviceExtensionSupport(physDevice);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = vulkanSwapChain.QuerySwapChainSupport(physDevice, vulkanWindow.GetSurface());
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    } else {
        return 0;
    }

    if (!swapChainAdequate) {
        return 0;
    }

    return score;
}

void VulkanRenderer::CreateVulkanInstance() {
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

void VulkanRenderer::PickPhysicalDevice() {
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

void VulkanRenderer::CreateLogicalDevice() {
    QueueFamilyIndices indices = FindQueueFamilies(physicalDevice, vulkanWindow.GetSurface());

    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos = VulkanQueue::GetQueueCreateInfos(indices);
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    VkPhysicalDeviceFeatures deviceFeatures = {};
    createInfo.pEnabledFeatures = &deviceFeatures;

    std::vector<const char*> deviceExtensions = VulkanSwapChain::GetDeviceExtensions();
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

    VulkanQueue::GetDeviceQueue(device, indices.graphicsFamily.value(), graphicsQueue);
    VulkanQueue::GetDeviceQueue(device, indices.presentFamily.value(), presentationQueue);
}

void VulkanRenderer::CreateRenderPass(const VulkanSwapChain& swapChain) {
    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = swapChain.GetFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}
