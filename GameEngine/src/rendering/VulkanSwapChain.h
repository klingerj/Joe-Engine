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
    std::vector<VkFramebuffer> swapChainFramebuffers;

public:
    VulkanSwapChain() : swapChain(), swapChainImages(), swapChainImageFormat(), swapChainExtent(),
                        swapChainImageViews(), swapChainFramebuffers() {}
    ~VulkanSwapChain() {}
    
    // Creates the swap chain
    void Create(const VkPhysicalDevice& physDevice, const VkDevice& device, const VkSurfaceKHR& surface, const int width, const int height);
    void CreateImageViews(const VkPhysicalDevice& physDevice, const VkDevice& device, const VkSurfaceKHR& surface, const int width, const int height);
    void CreateFramebuffers(const VkDevice& device, const VkRenderPass& renderPass);
    void Cleanup(const VkDevice& device);

    static std::vector<const char*> GetDeviceExtensions() {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    // Swap chain support checking/choosing
    bool CheckDeviceExtensionSupport(const VkPhysicalDevice& physDevice) const;
    SwapChainSupportDetails QuerySwapChainSupport(const VkPhysicalDevice& physDevice, const VkSurfaceKHR& surface) const;
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const uint32_t width, const uint32_t height) const;

    VkExtent2D GetExtent() const {
        return swapChainExtent;
    }
    VkFormat GetFormat() const {
        return swapChainImageFormat;
    }
};
