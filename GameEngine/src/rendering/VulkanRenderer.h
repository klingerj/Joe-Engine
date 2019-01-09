#pragma once

#include <optional>

#include "vulkan/vulkan.h"

#include "VulkanWindow.h"
#include "VulkanSwapChain.h"
#include "../utils/Common.h"
#include "../utils/VulkanValidationLayers.h"

namespace JoeEngine {
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
        ::std::string filepath = ""; // Path to custom shader if not using a built-in.
    } JEPostProcessingPass;

    // Class that manages all Vulkan resources and rendering

    class JEVulkanRenderer {
    private:
        // Wrapper for GLFW window
        JEVulkanWindow m_vulkanWindow;

        // Wrapper for Validation Layers
        JEVulkanValidationLayers m_vulkanValidationLayers;

        // Application width and height
        uint32_t m_width;
        uint32_t m_height;

        // Scene Manager
        JESceneManager* m_sceneManager;

        // Vulkan Instance creation
        VkInstance m_instance;

        // Vulkan physical/logical devices
        VkPhysicalDevice m_physicalDevice;
        VkDevice m_device;

        // Vulkan Queue(s)
        JEVulkanQueue m_graphicsQueue;
        JEVulkanQueue m_presentationQueue;

        // Swap chain
        JEVulkanSwapChain m_vulkanSwapChain;
        bool m_didFramebufferResize;

        // Framebuffers
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        // Command pool(s) and buffer(s)
        VkCommandPool m_commandPool;
        std::vector<VkCommandBuffer> m_commandBuffers;

        // Semaphores and Fences
        size_t m_currentFrame;
        const int m_MAX_FRAMES_IN_FLIGHT;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;

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
        JEOffscreenShadowPass m_shadowPass;
        void CreateShadowPassResources();
        void CreateShadowRenderPass();
        void CreateShadowFramebuffer();
        void CreateShadowCommandBuffer();

        // Deferred Rendering - geometry pass
        JEOffscreenDeferredPass m_deferredPass;
        void CreateDeferredPassGeometryResources();
        void CreateDeferredPassGeometryRenderPass();
        void CreateDeferredPassGeometryFramebuffer();
        void CreateDeferredPassGeometryCommandBuffer();

        // Deferred Rendering - lighting pass (only render offscreen if there is at least one post process)
        JEFramebufferAttachment m_framebufferAttachment_deferredLighting;
        VkRenderPass m_renderPass_deferredLighting;
        VkFramebuffer m_framebuffer_deferredLighting;
        void CreateDeferredPassLightingRenderPass();
        void CreateDeferredPassLightingFramebuffer();
        void CreateDeferredPassLightingResources();

        // Post processing
        // The final post processing pass's framebuffer attachment is never created (use the swap chain framebuffers instead)
        std::vector<JEPostProcessingPass> m_postProcessingPasses;
        void CreatePostProcessingPassResources();
        void CreatePostProcessingPassRenderPass(uint32_t i);
        void CreatePostProcessingPassFramebuffer(uint32_t i);

        void CreateDeferredLightingAndPostProcessingCommandBuffer();

    public:
        JEVulkanRenderer() : m_width(JE_DEFAULT_SCREEN_WIDTH), m_height(JE_DEFAULT_SCREEN_HEIGHT), m_MAX_FRAMES_IN_FLIGHT(JE_DEFAULT_MAX_FRAMES_IN_FLIGHT),
            m_currentFrame(0), m_didFramebufferResize(false), m_sceneManager(nullptr) {}
        ~JEVulkanRenderer() {}

        // Vulkan setup
        void Initialize(JESceneManager* sceneManager);
        void RegisterCallbacks(JEIOHandler* ioHandler);

        // Vulkan cleanup
        void Cleanup();

        void FramebufferResized() { m_didFramebufferResize = true; }

        // Draw a frame
        void DrawFrame();

        // Getters
        const JEVulkanWindow& GetWindow() const {
            return m_vulkanWindow;
        }
        GLFWwindow* GetGLFWWindow() const {
            return m_vulkanWindow.GetWindow();
        }
        VkDevice GetDevice() const {
            return m_device;
        }
    };

    static void JEFramebufferResizeCallback(GLFWwindow* window, int width, int height);
}
