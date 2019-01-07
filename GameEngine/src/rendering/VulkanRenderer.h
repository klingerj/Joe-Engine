#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanSwapChain.h"
#include "../utils/Common.h"
#include "../utils/VulkanValidationLayers.h"

class SceneManager;
class IOHandler;

// Rendering-related structs

// Generic Framebuffer attachment
typedef struct framebuffer_attachment_t {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView imageView;
} FramebufferAttachment;

// Render pass information for a shadow pass (depth-only)
typedef struct offscreen_shadow_pass_t {
    uint32_t width = DEFAULT_SHADOW_MAP_WIDTH, height = DEFAULT_SHADOW_MAP_HEIGHT;
    VkFramebuffer framebuffer;
    FramebufferAttachment depth;
    VkRenderPass renderPass;
    VkSampler depthSampler;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE; // Semaphore used to synchronize between this and the next render pass
} OffscreenShadowPass;

// Render pass information for a deferred rendering pass (multiple g-buffers)
typedef struct offscreen_deferred_pass_t {
    uint32_t width = DEFAULT_SCREEN_WIDTH, height = DEFAULT_SCREEN_HEIGHT;
    VkFramebuffer framebuffer;
    FramebufferAttachment color;
    FramebufferAttachment normal;
    FramebufferAttachment depth;
    VkRenderPass renderPass;
    VkSampler sampler;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE; // Semaphore used to synchronize between this and the next render pass
} OffscreenDeferredPass;

// Render pass information for a post processing pass (blit)
typedef struct post_processing_pass_t {
    uint32_t width = DEFAULT_SCREEN_WIDTH, height = DEFAULT_SCREEN_HEIGHT;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    FramebufferAttachment texture;
    VkRenderPass renderPass;
    VkSampler sampler;
} PostProcessingPass;

// Class that manages all Vulkan resources and rendering

class VulkanRenderer {
private:
    // Wrapper for GLFW window
    VulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    VulkanValidationLayers vulkanValidationLayers;

    // Application width and height
    uint32_t width;
    uint32_t height;

    // Scene Manager
    SceneManager* sceneManager;

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

    // Depth buffer
    FramebufferAttachment depthBuffer;

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

    // Setup functions
    void CreateVulkanInstance();
    std::vector<const char*> GetRequiredExtensions();
    int RateDeviceSuitability(VkPhysicalDevice physicalDevice, const VulkanWindow& vulkanWindow);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();
    void CreateSwapChainFramebuffers();
    void CreateSemaphoresAndFences();

    // Swap chain recreation
    void CleanupSwapChain();
    void RecreateSwapChain();

    /// Rendering variables and functions

    // Helpers for offscreen rendering
    void CreateFramebufferAttachment(FramebufferAttachment& depth, VkExtent2D extent, VkImageUsageFlagBits usageBits, VkFormat format);
    void CreateFramebufferAttachmentSampler(VkSampler& sampler);

    // Shadow pass
    OffscreenShadowPass shadowPass;
    void CreateShadowPassResources();
    void CreateShadowRenderPass();
    void CreateShadowFramebuffer();
    void CreateDepthAttachment(FramebufferAttachment& depth, VkExtent2D extent, VkImageUsageFlagBits usageBits);
    void CreateShadowCommandBuffer();

    // Deferred Rendering - geometry pass
    OffscreenDeferredPass deferredPass;
    void CreateDeferredPassGeometryResources();
    void CreateDeferredPassGeometryRenderPass();
    void CreateDeferredPassGeometryFramebuffer();
    void CreateDeferredPassGeometryAttachment(FramebufferAttachment& attachment, VkExtent2D extent, VkImageUsageFlagBits usageBits, VkFormat format);
    void CreateDeferredPassGeometryCommandBuffer();
    
    // Deferred Rendering - lighting pass (only render offscreen if there is at least one post process)
    FramebufferAttachment framebufferAttachment_deferredLighting;
    VkRenderPass renderPass_deferredLighting;
    VkFramebuffer framebuffer_deferredLighting;
    void CreateDeferredPassLightingRenderPass();
    void CreateDeferredPassLightingFramebuffer();
    void CreateDeferredPassLightingResources();

    // Post processing
    // The final post processing pass's framebuffer attachment is never created (use the swap chain framebuffers instead)
    std::vector<PostProcessingPass> postProcessingPasses;
    void CreatePostProcessingPassResources();
    void CreatePostProcessingPassRenderPass(uint32_t i);
    void CreatePostProcessingPassFramebuffer(uint32_t i);

    void CreateDeferredLightingAndPostProcessingCommandBuffer();

public:
    VulkanRenderer() : width(DEFAULT_SCREEN_WIDTH), height(DEFAULT_SCREEN_HEIGHT), MAX_FRAMES_IN_FLIGHT(DEFAULT_MAX_FRAMES_IN_FLIGHT),
                       currentFrame(0), framebufferResized(false), sceneManager(nullptr) {}
    ~VulkanRenderer() {}

    // Vulkan setup
    void Initialize(SceneManager* sceneManager);
    void RegisterCallbacks(IOHandler* ioHandler);

    // Vulkan cleanup
    void Cleanup();
    
    void FramebufferResized() { framebufferResized = true; }

    // Draw a frame
    void DrawFrame();

    // Getters
    const VulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
    GLFWwindow* GetGLFWWindow() const {
        return vulkanWindow.GetWindow();
    }
    VkDevice GetDevice() const {
        return device;
    }
};

static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
