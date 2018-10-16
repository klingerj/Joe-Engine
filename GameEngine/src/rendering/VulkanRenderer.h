#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanShader.h"
#include "Texture.h"
#include "Camera.h"
#include "../utils/VulkanValidationLayers.h"

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Application width and height
    const uint32_t width;
    const uint32_t height;

    // Vulkan Instance creation
    VkInstance instance;

    // Vulkan physical/logical devices
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Vulkan Queue(s)
    VulkanQueue graphicsQueue;
    VulkanQueue presentationQueue;

    // Swap chain
    VulkanSwapChain vulkanSwapChain;
    bool framebufferResized;

    // Camera
    Camera camera;

    // Meshes
    std::vector<Mesh> meshes;
    void UpdateModelMatrices();

    // Textures
    std::vector<Texture> textures;

    // Shaders and rendering
    std::vector<VulkanShader> shaders;
    VkRenderPass renderPass;
    
    // Framebuffers
    std::vector<VkFramebuffer> swapChainFramebuffers;

    // Command pool(s) and buffer(s)
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    // Semaphores and Fences
    size_t currentFrame;
    const int MAX_FRAMES_IN_FLIGHT;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;

public:
    VulkanRenderer() : width(DEFAULT_SCREEN_WIDTH), height(DEFAULT_SCREEN_HEIGHT), MAX_FRAMES_IN_FLIGHT(DEFAULT_MAX_FRAMES_IN_FLIGHT),
                       currentFrame(0), framebufferResized(false), camera() {}
    ~VulkanRenderer() {}

    // Vulkan setup
    void Initialize();

    // Vulkan cleanup
    void Cleanup();
    void CleanupSwapChain();

    // Swap chain recreation
    void RecreateSwapChain();
    void FramebufferResized() { framebufferResized = true; }

    // Draw a frame
    void DrawFrame();

    // Setup functions
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();
    int RateDeviceSuitability(VkPhysicalDevice physicalDevice, const VulkanWindow& vulkanWindow);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateRenderPass(const VulkanSwapChain& swapChain);
    void CreateFramebuffers();
    void CreateCommandPool();
    void CreateCommandBuffers();
    void CreateSemaphoresAndFences();

    // Getters
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
    VkDevice GetDevice() const {
        return device;
    }
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto app = reinterpret_cast<VulkanRenderer*>(glfwGetWindowUserPointer(window));
    app->FramebufferResized();
}
