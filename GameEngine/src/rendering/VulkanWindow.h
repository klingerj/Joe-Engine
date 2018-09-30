#pragma once

// Wrapper class for GLFW Window and VkSurfaceKHR
// Note: GLFW clean-up happens in destructor

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class VulkanRenderer;

class VulkanWindow {
private:
    GLFWwindow* window;
    VkSurfaceKHR surface;
public:
    VulkanWindow() {}
    ~VulkanWindow() {}

    void Initialize(const int w, const int h, const std::string& n, const VkInstance& instance);
    void Cleanup(const VkInstance& instance);
    void SetupVulkanSurface(const VkInstance& instance);

    // Getters
    const VkSurfaceKHR& GetSurface() const {
        return surface;
    }
    GLFWwindow* GetWindow() const {
        return window;
    }
    
    // Somewhat useless wrapping, but makes calls to the equivalent GLFW functions (needed for main game/render loop)
    bool ShouldClose() const {
        return glfwWindowShouldClose(window);
    }
    void PollEvents() const {
        glfwPollEvents();
    }
    const char** GetRequiredInstanceExtensions(uint32_t* count) const {
        return glfwGetRequiredInstanceExtensions(count);
    }
    void SetFrameBufferCallback(GLFWframebuffersizefun framebufferResizeCallback) const {
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    void SetWindowUserPointer(VulkanRenderer* renderer) {
        glfwSetWindowUserPointer(window, renderer);
    }
    void AwaitMaximize(int* width, int* height) const {
        glfwGetFramebufferSize(window, width, height);
        glfwWaitEvents();
    }
};
