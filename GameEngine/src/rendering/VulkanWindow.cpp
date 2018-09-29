#include "VulkanWindow.h"

void VulkanWindow::Initialize(const int w, const int h, const std::string& n, const VkInstance& instance) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(w, h, n.c_str(), nullptr, nullptr);
}

void VulkanWindow::Cleanup(const VkInstance& instance) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VulkanWindow::SetupVulkanSurface(const VkInstance& instance) {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface!");
    }
}
