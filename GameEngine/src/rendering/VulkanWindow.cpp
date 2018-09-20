#include "VulkanWindow.h"

void VulkanWindow::SetupVulkanSurface(const VkInstance& instance) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}

// Should only be called once during Vulkan setup
void VulkanWindow::CleanupVulkanSurface(const VkInstance& instance) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
}
