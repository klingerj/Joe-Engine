#pragma once

#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"


// Callback for printing validation layers messages

static VKAPI_ATTR VkBool32 VKAPI_CALL JEDebugCallback(
    VkDebugReportFlagsEXT flags,
    VkDebugReportObjectTypeEXT objType,
    uint64_t obj,
    size_t location,
    int32_t code,
    const char* layerPrefix,
    const char* msg,
    void* userData) {

    std::cerr << "validation layer: " << msg << std::endl;

    return VK_FALSE;
}

// Wrapper class for validation layers in Vulkan

class JEVulkanValidationLayers {
private:
    VkDebugReportCallbackEXT callback;
    const std::vector<const char*> validationLayers;
    const bool enableValidationLayers;
    bool EnableLayers();
public:
    // Construction and setup
    JEVulkanValidationLayers() : validationLayers{ "VK_LAYER_LUNARG_standard_validation" }, enableValidationLayers(EnableLayers()) {}
    ~JEVulkanValidationLayers() {}
    void SetupDebugCallback(const VkInstance& instance);
    void DestroyDebugCallback(const VkInstance& instance);

    // Getters
    bool CheckValidationLayerSupport() const;
    const bool AreValidationLayersEnabled() const {
        return enableValidationLayers;
    }
    const std::vector<const char*>& GetValidationLayers() const {
        return validationLayers;
    }
};
