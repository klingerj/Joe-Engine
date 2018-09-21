#pragma once

#include "vulkan/vulkan.h"

#include <optional>
#include <set>
#include <vector>

class VulkanWindow;

// Queue Family Indices
struct QueueFamilyIndices {
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physDevice, VkSurfaceKHR surface);

class VulkanQueue {
protected:
    VkQueue queue;
public:
    VulkanQueue() : queue(VK_NULL_HANDLE) {}
    ~VulkanQueue() {}
    static void GetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, VulkanQueue& vulkanQueue);
    static std::vector<VkDeviceQueueCreateInfo> GetQueueCreateInfos(const QueueFamilyIndices indices);
};
