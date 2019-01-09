#pragma once

#include <string>
#include "../utils/Common.h"

namespace JoeEngine {
    class JEVulkanRenderer;

    // Wrapper class for GLFW Window and VkSurfaceKHR
    // Note: GLFW clean-up happens in destructor

    class JEVulkanWindow {
    private:
        GLFWwindow* m_window;
        VkSurfaceKHR m_surface;

    public:
        JEVulkanWindow() {}
        ~JEVulkanWindow() {}

        void Initialize(const int w, const int h, const std::string& n, VkInstance instance);
        void Cleanup(VkInstance instance);
        void SetupVulkanSurface(VkInstance instance);

        // Getters
        VkSurfaceKHR GetSurface() const {
            return m_surface;
        }
        GLFWwindow* GetWindow() const {
            return m_window;
        }

        // Somewhat useless wrapping, but makes calls to the equivalent GLFW functions (needed for main game/render loop)
        bool ShouldClose() const {
            return glfwWindowShouldClose(m_window);
        }
        const char** GetRequiredInstanceExtensions(uint32_t* count) const {
            return glfwGetRequiredInstanceExtensions(count);
        }
        void SetFrameBufferCallback(GLFWframebuffersizefun framebufferResizeCallback) const {
            glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
        }
        void AwaitMaximize(int* width, int* height) const {
            glfwGetFramebufferSize(m_window, width, height);
            glfwWaitEvents();
        }
    };
}
