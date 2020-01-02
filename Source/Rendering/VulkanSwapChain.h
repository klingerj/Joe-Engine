#pragma once

#include <vector>

#include "GLFW/glfw3.h"
#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanQueue.h"
#include "../Utils/Common.h"

namespace JoeEngine {
    //! Swap chain support detail data struct.
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    //! The JEVulkanSwapChain class.
    /*!
      Class that creates and manages data for the swap chain.
    */
    class JEVulkanSwapChain {
    private:
        //! The Vulkan swap chain object.
        VkSwapchainKHR m_swapChain;

        //! List of swap chain image data (one per element in the swap chain).
        std::vector<VkImage> m_swapChainImages;

        //! Swap chain image format.
        VkFormat m_swapChainImageFormat;

        //! Swap chain dimensions.
        VkExtent2D m_swapChainExtent;

        //! List of swap chain image view data (one per element in the swap chain).
        std::vector<VkImageView> m_swapChainImageViews;

        //! Choose swap chain surface format.
        /*!
          Choose a format for the swap chain based on the available formats.
          \param availableFormats list of available formats.
          \return the best available surface format.
        */
        VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;

        //! Choose swap chain present mode.
        /*!
          Choose a present mode for the swap chain based on available present modes.
          \param availablePresentModes list of available present modes.
          \return the best available present mode.
        */
        VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;

        //! Choose dimensions for the swap chain based on the device capabilities and the desired window dimensions.
        /*!
          \param capabilities the surface capabilities data (for swap chain extent capabilities)
          \param the GLFW window (for intended framebuffer size)
          \return the final dimensions of the swap chain.
        */
        VkExtent2D ChooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window) const;

        //! Create swap chain image views.
        /*!
          Creates the list of Vulkan image view objects for the swap chain.
          \param physicalDevice the Vulkan physicalDevice
          \param device the Vulkan logical device.
        */
        void CreateImageViews(VkPhysicalDevice physicalDevice, VkDevice device);

    public:
        //! Constructor (default).
        JEVulkanSwapChain() = default;

        //! Destructor (default).
        ~JEVulkanSwapChain() = default;

        //! Get extension support for the Vulkan physical device.
        /*!
          \param physicalDevice the Vulkan physical device to check for extension support with.
          \return true if the physical device supports all necessary extensions.
        */
        bool CheckDeviceExtensionSupport(VkPhysicalDevice physicalDevice) const;

        //! Get swap chain support details.
        /*!
          Returns a struct containing all data detailing the swap chain capabilities of the device.
          \param physicalDevice the Vulkan physical device to check support with.
          \param surface the Vulkan surface to check support with.
          \return struct containing swap chain support details.
        */
        SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) const;

        //! Create the swap chain object.
        /*!
          \param physicalDevice the Vulkan physical device.
          \param device the Vulkan logical device.
          \param vulkanWindow the JEVulkanWindow object.
          \param width the intended swap chain width.
          \param height the intended swap chain height.
        */
        void Create(VkPhysicalDevice physicalDevice, VkDevice device, const JEVulkanWindow& vulkanWindow, uint32_t width, uint32_t height);
        
        //! Cleanup memory.
        //! \param device the Vulkan logical device needed for cleanup.
        void Cleanup(VkDevice device);

        //! Get necessary swap chain device extension name(s).
        //! \return list of swap chain device extension name(s).
        static std::vector<const char*> GetDeviceExtensions() {
            return { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
        }

        //! Get swap chain object.
        //! \return swap chain object.
        VkSwapchainKHR GetSwapChain() const {
            return m_swapChain;
        }

        //! Get swap chain extent.
        //! \return swap chain extent (dimensions).
        VkExtent2D GetExtent() const {
            return m_swapChainExtent;
        }

        //! Get swap chain format.
        //! \return the swap chain image format.
        VkFormat GetFormat() const {
            return m_swapChainImageFormat;
        }

        //! Get list of swap chain image views.
        //! \return list of swap chain image view objects.
        const std::vector<VkImageView>& GetImageViews() const {
            return m_swapChainImageViews;
        }
    };
}
