#pragma once

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "../VulkanValidationLayers.h"
#include "VulkanQueue.h"
#include "VulkanSwapChain.h"
#include "..\GlobalInfo.h"

// Wrapper for Vulkan physical/logical device, window, and swap chain

class VulkanDevice {
protected:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Application width and height
    uint32_t width;
    uint32_t height;

    // Vulkan Instance creation
    VkInstance instance;
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();
    int RateDeviceSuitability(const VkPhysicalDevice& physDevice, const VulkanWindow& vulkanWindow);

    // Vulkan physical/logical devices and creation functions
    VkPhysicalDevice physicalDevice;
    void PickPhysicalDevice();
    VkDevice device;
    void CreateLogicalDevice();

    // Vulkan Queue (wrapper class)
    VulkanQueue graphicsQueue;
    VulkanQueue presentationQueue;

    // Swap chain
    VulkanSwapChain vulkanSwapChain;

public:
    VulkanDevice() : vulkanWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "VulkanWindow"), width(DEFAULT_SCREEN_WIDTH), height(DEFAULT_SCREEN_HEIGHT) {
        CreateVulkanInstance();
        vulkanValidationLayers.SetupDebugCallback(instance);
        vulkanWindow.SetupVulkanSurface(instance);
        PickPhysicalDevice();
        CreateLogicalDevice();
        vulkanSwapChain.Create(physicalDevice, device, vulkanWindow.GetSurface(), width, height);
    }

    ~VulkanDevice() {
        vulkanSwapChain.Cleanup(device);
        
        vkDestroyDevice(device, nullptr);
        if (vulkanValidationLayers.AreValidationLayersEnabled()) {
            vulkanValidationLayers.DestroyDebugCallback(instance);
        }
        vulkanWindow.CleanupVulkanSurface(instance);
        vkDestroyInstance(instance, nullptr);
    }
    
    // Getters
    const VkInstance& GetInstance() const {
        return instance;
    }
    const VkDevice& GetDevice() const {
        return device;
    }
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
    const VulkanSwapChain& GetSwapChain() const {
        return vulkanSwapChain;
    }
};
