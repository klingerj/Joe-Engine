#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanShader.h"
#include "../VulkanValidationLayers.h"
#include "../GlobalInfo.h"

#define NUM_VULKAN_SHADERS 1

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Application width and height
    uint32_t width;
    uint32_t height;

    // Vulkan Instance creation
    VkInstance instance;

    // Vulkan physical/logical devices
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Vulkan Queue(s)
    VulkanQueue graphicsQueue;
    VulkanQueue presentationQueue;

    // Swap chain
    VulkanSwapChain vulkanSwapChain;

    // Shaders and rendering
    VulkanShader shaders[NUM_VULKAN_SHADERS];
    VkRenderPass renderPass;

public:
    VulkanRenderer() {}
    ~VulkanRenderer() {}

    void Initialize();
    void Cleanup();

    // Setup functions
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();
    int RateDeviceSuitability(const VkPhysicalDevice& physDevice, const VulkanWindow& vulkanWindow);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateRenderPass(const VulkanSwapChain& swapChain);

    // Getters
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
};
