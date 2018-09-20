#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanDevice.h"

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for Vulkan physical/logical device, window, and swap chain
    VulkanDevice vulkanDevice;

public:
    VulkanRenderer() : vulkanDevice() {}
    ~VulkanRenderer() {}

    const VulkanDevice& GetDevice() {
        return vulkanDevice;
    }
};
