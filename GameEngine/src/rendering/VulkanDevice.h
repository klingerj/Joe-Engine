#pragma once

#include "vulkan/vulkan.h"

#include "../VulkanValidationLayers.h"
#include "VulkanQueue.h"
#include "VulkanWindow.h"

// Wrapper for Vulkan physical/logical device, window, and swap chain

class VulkanDevice {
protected:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Vulkan Instance creation
    VkInstance instance;
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Vulkan physical/logical devices and creation functions
    VkPhysicalDevice physicalDevice;
    void PickPhysicalDevice();
    VkDevice device;
    void CreateLogicalDevice();

    // Vulkan Queue (wrapper class)
    VulkanQueue graphicsQueue;
    VulkanQueue presentationQueue;

    // Swap chain


public:
    VulkanDevice() : vulkanWindow(800, 600, "VulkanWindow"), graphicsQueue(), presentationQueue() {
        CreateVulkanInstance();
        vulkanValidationLayers.SetupDebugCallback(instance);
        vulkanWindow.SetupVulkanSurface(instance);
        PickPhysicalDevice();
        CreateLogicalDevice();
    }
    ~VulkanDevice() {
        vkDestroyDevice(device, nullptr);
        if (VulkanValidationLayers::AreValidationLayersEnabled()) {
            vulkanValidationLayers.DestroyDebugCallback(instance);
        }
        vulkanWindow.CleanupVulkanSurface(instance);
        vkDestroyInstance(instance, nullptr);
    }

    const VkInstance& GetInstance() const {
        return instance;
    }

    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
};
