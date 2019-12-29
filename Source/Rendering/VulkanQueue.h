#pragma once

#include "vulkan/vulkan.h"

#include <optional>
#include <set>
#include <vector>

namespace JoeEngine {
    class JEVulkanWindow;

    //! Queue family indices data struct.
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    //! Find queue families.
    /*!
      Finds which queue families the Vulkan physical device supports.
      \param physicalDevice the Vulkan physical device to check for queue support with.
      \param surface the Vulkan surface to check for queue support with.
      \return a QueueFamilyIndices struct containing the necessary queue support info.
    */
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

    //! Get queue create info data.
    /*!
      Returns device queue create info structs given queue family indices.
      \param indices the given queue family indices data.
      \return list of device queue create info structs for each supported queue family.
    */
    std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfos(const QueueFamilyIndices& indices);

    //! The JEVulkanQueue class.
    /*!
      Class that manages Vulkan queue data and provides some convenience functions.
    */
    class JEVulkanQueue {
    private:
        //! Vulkan queue object.
        VkQueue m_queue;

    public:
        //! Constructor.
        //! Initializes Vulkan member variable to null handle.
        JEVulkanQueue() : m_queue(VK_NULL_HANDLE) {}

        //! Destructor (default).
        ~JEVulkanQueue() = default;

        //! Get device queue.
        /*!
          Wrapper around Vulkan function that gets a device queue given a logical device and queue family index.
          \param device the Vulkan logical device to get a queue from.
          \param queueFamilyIndex the specific queue index to get from the device.
        */
        void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex);

        //! Get the Vulkan queue object.
        //! \return the Vulkan queue object.
        VkQueue GetQueue() const {
            return m_queue;
        }
    };
}
