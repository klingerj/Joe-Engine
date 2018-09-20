#pragma once

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "../VulkanValidationLayers.h"

class VulkanRenderer {
private:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Vulkan Instance creation
    VkInstance instance;
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();

public:
    VulkanRenderer() : vulkanWindow(800, 600, "VulkanWindow") {
        CreateVulkanInstance();
        vulkanValidationLayers.SetupDebugCallback(instance);
    }

    ~VulkanRenderer() {
        vkDestroyInstance(instance, nullptr);
        vulkanValidationLayers.DestroyDebugCallback(instance);
    }

    // Get the VulkanWindow
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
};
