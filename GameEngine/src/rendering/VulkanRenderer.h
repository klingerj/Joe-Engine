#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanDevice.h"
#include "VulkanShader.h"
#include "..\GlobalInfo.h"

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for Vulkan physical/logical device, window, and swap chain
    //VulkanDevice vulkanDevice;

    // Graphics stuff
    VulkanShader shaders[1];

public:
    VulkanRenderer(const VulkanDevice& vulkanDevice) :
                   shaders{ VulkanShader(&vulkanDevice.GetDevice(), vulkanDevice.GetSwapChain(),
                   SHADER_DIRECTORY + "vert_basic.spv", SHADER_DIRECTORY + "frag_basic.spv") } {}
    ~VulkanRenderer() {}

    //void Initialize(const VulkanDevice& VulkanDevice);
};
