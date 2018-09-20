#pragma once

#include <vector>
#include <iostream>

#include "vulkan/vulkan.h"


// Callback for printing validation layers messages

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {

    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;
}


// Wrapper class for validation layers in Vulkan

class VulkanValidationLayers {
private:
    VkDebugUtilsMessengerEXT callback;
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT callback, const VkAllocationCallbacks* pAllocator);
public:
    static const std::vector<const char*> validationLayers;
    static const bool enableValidationLayers;

    // Construction and setup
    VulkanValidationLayers() {}
    ~VulkanValidationLayers() {}
    void SetupDebugCallback(const VkInstance& instance);
    void DestroyDebugCallback(const VkInstance& instance);

    // Getters
    bool CheckValidationLayerSupport() const;
    bool AreValidationLayersEnabled() const {
        return enableValidationLayers;
    }
    const std::vector<const char*>& GetValidationLayers() const {
        return validationLayers;
    }
};
