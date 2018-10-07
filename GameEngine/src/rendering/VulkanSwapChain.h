#pragma once

#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanQueue.h"
#include "../Common.h"

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
    VulkanSwapChain() {}
    ~VulkanSwapChain() {}
    
    // Creates the swap chain
    void Create(VkPhysicalDevice physicalDevice, VkDevice device, const VulkanWindow& vulkanWindow, int width, int height);
    void CreateImageViews(VkPhysicalDevice physicalDevice, VkDevice device, VkSurfaceKHR surface, int width, int height);
    void Cleanup(VkDevice device);

    static std::vector<const char*> GetDeviceExtensions() {
        return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    }

    // Swap chain support checking/choosing
    bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const;
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;
    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window) const;

    VkSwapchainKHR GetSwapChain() const {
        return swapChain;
    }
    VkExtent2D GetExtent() const {
        return swapChainExtent;
    }
    VkFormat GetFormat() const {
        return swapChainImageFormat;
    }
    const std::vector<VkImageView>& GetImageViews() const {
        return swapChainImageViews;
    }
};
