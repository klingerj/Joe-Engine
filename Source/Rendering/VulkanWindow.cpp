#include "VulkanWindow.h"

namespace JoeEngine {
    void JEVulkanWindow::Initialize(const int w, const int h, const std::string& n) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        m_window = glfwCreateWindow(w, h, n.c_str(), nullptr, nullptr);
    }

    void JEVulkanWindow::Cleanup(VkInstance instance) {
        vkDestroySurfaceKHR(instance, m_surface, nullptr);
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    void JEVulkanWindow::SetupVulkanSurface(VkInstance instance) {
        if (glfwCreateWindowSurface(instance, m_window, nullptr, &m_surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }
}
