#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanDevice.h"

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for Vulkan physical/logical device, window, and swap chain
    VulkanDevice vulkanDevice;

    // Graphics stuff


public:
    VulkanRenderer() : vulkanDevice() {
        CreateGraphicsPipeline();
    }
    ~VulkanRenderer() {}

    // Vulkan Creation
    void CreateGraphicsPipeline();

    // Getters
    const VulkanDevice& GetDevice() {
        return vulkanDevice;
    }
};
