#pragma once

#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanQueue.h"
#include "../utils/Common.h"

namespace JoeEngine {
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class JEVulkanSwapChain {
    private:
        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;
        std::vector<VkImageView> m_swapChainImageViews;

    public:
        JEVulkanSwapChain() {}
        ~JEVulkanSwapChain() {}

        // Creates the swap chain
        void Create(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanWindow& vulkanWindow, int width, int height);
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
            return m_swapChain;
        }
        VkExtent2D GetExtent() const {
            return m_swapChainExtent;
        }
        VkFormat GetFormat() const {
            return m_swapChainImageFormat;
        }
        const std::vector<VkImageView>& GetImageViews() const {
            return m_swapChainImageViews;
        }
    };
}
