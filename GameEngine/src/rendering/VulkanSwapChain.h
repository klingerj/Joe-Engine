#pragma once

#include <vector>

#include "vulkan/vulkan.h"

class VulkanSwapChain {
public:
    VulkanSwapChain() {}
    
    static std::vector<const char*> GetDeviceExtensions() {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }
};
