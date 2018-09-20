#pragma once

// Wrapper class for GLFW Window
// Note: GLFW clean-up happens in destructor

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>

class VulkanWindow {
private:
    GLFWwindow* window;
public:
    VulkanWindow(const int w, const int h, const std::string& n) {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(w, h, n.c_str(), nullptr, nullptr);
    }

    ~VulkanWindow() {
        glfwDestroyWindow(window);
        glfwTerminate();
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
};
