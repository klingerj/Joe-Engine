#pragma once

#include <vector>

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanQueue.h"

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class VulkanSwapChain {
private:
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
public:
    VulkanSwapChain() : swapChain() {}
    ~VulkanSwapChain() {}
    
    // Creates the swap chain
    void Create(const VkPhysicalDevice& physDevice, const VkDevice& device, const VkSurfaceKHR& surface, const int width, const int height);
    void CreateImageViews(const VkPhysicalDevice& physDevice, const VkDevice& device, const VkSurfaceKHR& surface, const int width, const int height);
    void Cleanup(const VkDevice& device);

    static std::vector<const char*> GetDeviceExtensions() {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    // Swap chain support checking/choosing
    bool CheckDeviceExtensionSupport(VkPhysicalDevice physDevice) const;
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physDevice, const VkSurfaceKHR& surface) const;
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) const;
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t width, const uint32_t height) const;
};
