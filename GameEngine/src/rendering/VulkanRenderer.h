#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanSwapChain.h"
#include "../utils/Common.h"
#include "../utils/VulkanValidationLayers.h"

class JESceneManager;
class JEIOHandler;

// Rendering-related structs

// Generic Framebuffer attachment
typedef struct je_framebuffer_attachment_t {
    VkImage image;
    VkDeviceMemory deviceMemory;
    VkImageView imageView;
} JEFramebufferAttachment;

// Render pass information for a shadow pass (depth-only)
typedef struct je_offscreen_shadow_pass_t {
    uint32_t width = JE_DEFAULT_SHADOW_MAP_WIDTH, height = JE_DEFAULT_SHADOW_MAP_HEIGHT;
    VkFramebuffer framebuffer;
    JEFramebufferAttachment depth;
    VkRenderPass renderPass;
    VkSampler depthSampler;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE; // Semaphore used to synchronize between this and the next render pass
} JEOffscreenShadowPass;

// Render pass information for a deferred rendering pass (multiple g-buffers)
typedef struct je_offscreen_deferred_pass_t {
    uint32_t width = JE_DEFAULT_SCREEN_WIDTH, height = JE_DEFAULT_SCREEN_HEIGHT;
    VkFramebuffer framebuffer;
    JEFramebufferAttachment color;
    JEFramebufferAttachment normal;
    JEFramebufferAttachment depth;
    VkRenderPass renderPass;
    VkSampler sampler;
    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkSemaphore semaphore = VK_NULL_HANDLE; // Semaphore used to synchronize between this and the next render pass
} JEOffscreenDeferredPass;

// Render pass information for a post processing pass (blit)
typedef struct je_post_processing_pass_t {
    uint32_t width = JE_DEFAULT_SCREEN_WIDTH, height = JE_DEFAULT_SCREEN_HEIGHT;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    JEFramebufferAttachment texture;
    VkRenderPass renderPass;
    VkSampler sampler;
    uint32_t shaderIndex = -1; // ID indicating which built-in post shader to use. -1 for custom shader.
    std::string filepath = ""; // Path to custom shader if not using a built-in.
} JEPostProcessingPass;

// Class that manages all Vulkan resources and rendering

class JEVulkanRenderer {
private:
    // Wrapper for GLFW window
    JEVulkanWindow vulkanWindow;

    // Wrapper for Validation Layers
    JEVulkanValidationLayers vulkanValidationLayers;

    // Application width and height
    uint32_t width;
    uint32_t height;

    // Scene Manager
    JESceneManager* sceneManager;

    // Vulkan Instance creation
    VkInstance instance;

    // Vulkan physical/logical devices
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    // Vulkan Queue(s)
    JEVulkanQueue graphicsQueue;
    JEVulkanQueue presentationQueue;

    // Swap chain
    JEVulkanSwapChain vulkanSwapChain;
    bool framebufferResized;

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
    int RateDeviceSuitability(VkPhysicalDevice physicalDevice, const JEVulkanWindow& vulkanWindow);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();
    void CreateSwapChainFramebuffers();
    void CreateSemaphoresAndFences();

    // Window-dependent rendering resource recreation
    void CleanupWindowDependentRenderingResources();
    void RecreateWindowDependentRenderingResources();

    /// Rendering variables and functions

    // Helpers for offscreen rendering
    void CreateFramebufferAttachment(JEFramebufferAttachment& depth, VkExtent2D extent, VkImageUsageFlagBits usageBits, VkFormat format);
    void CreateFramebufferAttachmentSampler(VkSampler& sampler);

    // Shadow pass
    JEOffscreenShadowPass shadowPass;
    void CreateShadowPassResources();
    void CreateShadowRenderPass();
    void CreateShadowFramebuffer();
    void CreateShadowCommandBuffer();

    // Deferred Rendering - geometry pass
    JEOffscreenDeferredPass deferredPass;
    void CreateDeferredPassGeometryResources();
    void CreateDeferredPassGeometryRenderPass();
    void CreateDeferredPassGeometryFramebuffer();
    void CreateDeferredPassGeometryCommandBuffer();
    
    // Deferred Rendering - lighting pass (only render offscreen if there is at least one post process)
    JEFramebufferAttachment framebufferAttachment_deferredLighting;
    VkRenderPass renderPass_deferredLighting;
    VkFramebuffer framebuffer_deferredLighting;
    void CreateDeferredPassLightingRenderPass();
    void CreateDeferredPassLightingFramebuffer();
    void CreateDeferredPassLightingResources();

    // Post processing
    // The final post processing pass's framebuffer attachment is never created (use the swap chain framebuffers instead)
    std::vector<JEPostProcessingPass> postProcessingPasses;
    void CreatePostProcessingPassResources();
    void CreatePostProcessingPassRenderPass(uint32_t i);
    void CreatePostProcessingPassFramebuffer(uint32_t i);

    void CreateDeferredLightingAndPostProcessingCommandBuffer();

public:
    JEVulkanRenderer() : width(JE_DEFAULT_SCREEN_WIDTH), height(JE_DEFAULT_SCREEN_HEIGHT), MAX_FRAMES_IN_FLIGHT(JE_DEFAULT_MAX_FRAMES_IN_FLIGHT),
                       currentFrame(0), framebufferResized(false), sceneManager(nullptr) {}
    ~JEVulkanRenderer() {}

    // Vulkan setup
    void Initialize(JESceneManager* sceneManager);
    void RegisterCallbacks(JEIOHandler* ioHandler);

    // Vulkan cleanup
    void Cleanup();
    
    void FramebufferResized() { framebufferResized = true; }

    // Draw a frame
    void DrawFrame();

    // Getters
    const JEVulkanWindow& GetWindow() const {
        return vulkanWindow;
    }
    GLFWwindow* GetGLFWWindow() const {
        return vulkanWindow.GetWindow();
    }
    VkDevice GetDevice() const {
        return device;
    }
};

static void JEFramebufferResizeCallback(GLFWwindow* window, int width, int height);
