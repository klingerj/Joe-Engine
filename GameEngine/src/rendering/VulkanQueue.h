#pragma once

#include "vulkan/vulkan.h"

#include <optional>
#include <set>
#include <vector>

namespace JoeEngine {
    class JEVulkanWindow;

    // Queue Family Indices
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool IsComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);
    std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfos(const QueueFamilyIndices& indices);

    class JEVulkanQueue {
    private:
        VkQueue m_queue;

    public:
        JEVulkanQueue() : m_queue(VK_NULL_HANDLE) {}
        ~JEVulkanQueue() {}

        // Setup
        void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex);

        // Getters
        VkQueue GetQueue() const {
            return m_queue;
        }
    };
}
