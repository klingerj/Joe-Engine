#pragma once

// Wrapper class for GLFW Window and VkSurfaceKHR
// Note: GLFW clean-up happens in destructor

#include <string>
#include "../utils/Common.h"

class JEVulkanRenderer;

class JEVulkanWindow {
private:
    GLFWwindow* window;
    VkSurfaceKHR surface;

public:
    JEVulkanWindow() {}
    ~JEVulkanWindow() {}

    void Initialize(const int w, const int h, const std::string& n, VkInstance instance);
    void Cleanup(VkInstance instance);
    void SetupVulkanSurface(VkInstance instance);

    // Getters
    VkSurfaceKHR GetSurface() const {
        return surface;
    }
    GLFWwindow* GetWindow() const {
        return window;
    }
    
    // Somewhat useless wrapping, but makes calls to the equivalent GLFW functions (needed for main game/render loop)
    bool ShouldClose() const {
        return glfwWindowShouldClose(window);
    }
    const char** GetRequiredInstanceExtensions(uint32_t* count) const {
        return glfwGetRequiredInstanceExtensions(count);
    }
    void SetFrameBufferCallback(GLFWframebuffersizefun framebufferResizeCallback) const {
        glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
    }
    void AwaitMaximize(int* width, int* height) const {
        glfwGetFramebufferSize(window, width, height);
        glfwWaitEvents();
    }
};
