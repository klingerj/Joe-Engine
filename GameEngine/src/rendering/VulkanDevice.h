#pragma once

#include "vulkan/vulkan.h"

#include "../VulkanValidationLayers.h"
#include "VulkanQueue.h"
#include "VulkanWindow.h"
#include "VulkanSwapChain.h"


#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600

// Wrapper for Vulkan physical/logical device, window, and swap chain

class VulkanDevice {
protected:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Application width and height
    uint32_t width;
    uint32_t height;

    // Vulkan Instance creation
    VkInstance instance;
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();
    int RateDeviceSuitability(VkPhysicalDevice physDevice, const VulkanWindow& vulkanWindow);

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
    VulkanSwapChain swapChain;

public:
    VulkanDevice() : vulkanWindow(DEFAULT_SCREEN_WIDTH, DEFAULT_SCREEN_HEIGHT, "VulkanWindow"), width(DEFAULT_SCREEN_WIDTH), height(DEFAULT_SCREEN_HEIGHT),
                     instance(), vulkanValidationLayers(), physicalDevice(), device(), graphicsQueue(), presentationQueue(), swapChain() {
        CreateVulkanInstance();
        vulkanValidationLayers.SetupDebugCallback(instance);
        vulkanWindow.SetupVulkanSurface(instance);
        PickPhysicalDevice();
        CreateLogicalDevice();
        swapChain.Create(physicalDevice, device, vulkanWindow.GetSurface(), width, height);
    }

    ~VulkanDevice() {
        swapChain.Cleanup(device);
        vkDestroyDevice(device, nullptr);
        if (VulkanValidationLayers::AreValidationLayersEnabled()) {
            vulkanValidationLayers.DestroyDebugCallback(instance);
        }
        vulkanWindow.CleanupVulkanSurface(instance);
        vkDestroyInstance(instance, nullptr);
    }
    
    // Getters
    const VkInstance& GetInstance() const {
        return instance;
    }
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
};
