#pragma once

#include <string>
#include <functional>

//! Tell GLFW to use Vulkan (https://www.glfw.org/docs/latest/vulkan_guide.html).
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

//! Force GLM to use radians
#define GLM_FORCE_RADIANS

//! Force GLM to keep depth values on the range [0, 1]
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

//! Enable experimental features of GLM
#define GLM_ENABLE_EXPERIMENTAL

//! Force GLM to use left-handed coordinate systems
#define GLM_FORCE_LEFT_HANDED

#include "vulkan/vulkan.h"
#include "../Rendering/VulkanQueue.h"
#include "glm/glm.hpp"

namespace JoeEngine {
    // Constants for rendering

    //! Default screen resolution width.
    constexpr int JE_DEFAULT_SCREEN_WIDTH = 1280;

    //! Default screen resolution height.
    constexpr int JE_DEFAULT_SCREEN_HEIGHT = 720;

    //! Default maximum number of frames in flight on the GPU.
    constexpr uint32_t JE_DEFAULT_MAX_FRAMES_IN_FLIGHT = 2;

    //! Maximum number of entities in any given scene.
    constexpr int JE_NUM_ENTITIES = 10000;

    //! Maximum number of fragments per pixel for OIT.
    constexpr uint16_t JE_NUM_OIT_FRAGSPP = 16;

    //! Default shadow map resolution width.
    constexpr int JE_DEFAULT_SHADOW_MAP_WIDTH = 4000;

    //! Default shadow map resolution height.
    constexpr int JE_DEFAULT_SHADOW_MAP_HEIGHT = 4000;

    //! Default shadow map depth bias - slope.
    constexpr float JE_DEFAULT_SHADOW_MAP_DEPTH_BIAS_SLOPE = 1.5f;

    //! Default shadow map depth bias - constant.
    constexpr float JE_DEFAULT_SHADOW_MAP_DEPTH_BIAS_CONSTANT = 0.0f;

    // Camera attributes

    //! World up vector (Y is up).
    const glm::vec3 JE_WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);

    //! Scene camera near clip plane value.
    constexpr float JE_SCENE_VIEW_NEAR_PLANE = 0.1f;

    //! Scene camera far clip plane value.
    constexpr float JE_SCENE_VIEW_FAR_PLANE = 200.0f;

    //! Shadow camera near clip plane value.
    constexpr float JE_SHADOW_VIEW_NEAR_PLANE = 0.1f;

    //! Shadow camera far clip plane value.
    constexpr float JE_SHADOW_VIEW_FAR_PLANE = 200.0f;

    //! Scene camera FOV value.
    const float JE_FOVY = glm::radians(22.5f);

    // Engine settings

    //! Engine renderer settings bit flag.
    typedef enum class JE_RENDERER_SETTINGS_TYPE : uint32_t {
        Default = 0x0,
        EnableDeferred = 0x1,
        EnableOIT = 0x2,
        AllSettings = 0xFFFFFFFF
    } RendererSettings;

    //! &-operator for Renderer Settings.
    inline bool operator&(RendererSettings a, RendererSettings b) { // Note the pass by value
        return (uint32_t)a & (uint32_t)b;
    }

    //! |- operator for Renderer Settings.
    inline RendererSettings operator|(RendererSettings a, RendererSettings b) { // Note the pass by value
        return (RendererSettings)((uint32_t)a | (uint32_t)b);
    }

    // Vulkan Functions

    //! Begin single time command buffer recording.
    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);

    //! End single time command buffer recording.
    void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, const JEVulkanQueue& graphicsQueue, VkCommandPool commandPool);

    //! Check if a depth format has a stencil component.
    bool HasStencilComponent(VkFormat format);

    //! Get memory for an allocation.
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);

    //! Create buffer on the GPU.
    void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    
    //! Copy buffer CPU to GPU.
    void CopyBuffer(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    
    //! Create image.
    void CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    //! Create image view.s
    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    
    //! Transition image from initial to final layout.
    void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    
    //! Check if several candidate formats are supported.
    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    
    //! Find a supported format among several candidate depth formats.
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

    //! Simple typedef for std::function.
    using JECallbackFunction = std::function<void()>;

    //! Project path string.
    extern const std::string JE_PROJECT_PATH;

    //! Shader directory path string.
    extern const std::string JE_SHADER_DIR;

    //! OBJ models path string.
    extern const std::string JE_MODELS_OBJ_DIR;

    //! Textures directory path string.
    extern const std::string JE_TEXTURES_DIR;

    // Post Processing Shaders
    // The Joe Engine provides some built-in post processing shaders, distinguished by index in this array.
    // (Now deprecated temporarily)
    //extern const std::string JEBuiltInPostProcessingShaderPaths[2];

    // IO - defines for keys

    //! Keypress - W
    const int JE_KEY_W = GLFW_KEY_W;

    //! Keypress - A
    const int JE_KEY_A = GLFW_KEY_A;

    //! Keypress - S
    const int JE_KEY_S = GLFW_KEY_S;

    //! Keypress - D
    const int JE_KEY_D = GLFW_KEY_D;

    //! Keypress - Q
    const int JE_KEY_Q = GLFW_KEY_Q;

    //! Keypress - E
    const int JE_KEY_E = GLFW_KEY_E;

    //! Keypress - Up arrow
    const int JE_KEY_UP = GLFW_KEY_UP;

    //! Keypress - Left arrow
    const int JE_KEY_LEFT = GLFW_KEY_LEFT;

    //! Keypress - Down arrow
    const int JE_KEY_DOWN = GLFW_KEY_DOWN;

    //! Keypress - Right arrow
    const int JE_KEY_RIGHT = GLFW_KEY_RIGHT;

    //! Keypress - 0
    const int JE_KEY_0 = GLFW_KEY_0;

    //! Keypress - 1
    const int JE_KEY_1 = GLFW_KEY_1;

    //! Keypress - 2
    const int JE_KEY_2 = GLFW_KEY_2;

    //! Keypress - 3
    const int JE_KEY_3 = GLFW_KEY_3;

    //! Keypress - 4
    const int JE_KEY_4 = GLFW_KEY_4;

    //! Keypress - 5
    const int JE_KEY_5 = GLFW_KEY_5;

    //! Keypress - 6
    const int JE_KEY_6 = GLFW_KEY_6;

    //! Keypress - 7
    const int JE_KEY_7 = GLFW_KEY_7;

    //! Keypress - 8
    const int JE_KEY_8 = GLFW_KEY_8;

    //! Keypress - 9
    const int JE_KEY_9 = GLFW_KEY_9;
}
