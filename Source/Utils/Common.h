#pragma once

#include <string>
#include <functional>

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED

#include "vulkan/vulkan.h"
#include "../Rendering/VulkanQueue.h"
#include "glm/glm.hpp"

namespace JoeEngine {
    // Constants for rendering
    constexpr int JE_DEFAULT_SCREEN_WIDTH = 1280;
    constexpr int JE_DEFAULT_SCREEN_HEIGHT = 720;
    constexpr int JE_DEFAULT_MAX_FRAMES_IN_FLIGHT = 2;

    constexpr int JE_DEFAULT_SHADOW_MAP_WIDTH = 2000;
    constexpr int JE_DEFAULT_SHADOW_MAP_HEIGHT = 2000;
    constexpr float JE_DEFAULT_SHADOW_MAP_DEPTH_BIAS_SLOPE = 1.0f;
    constexpr float JE_DEFAULT_SHADOW_MAP_DEPTH_BIAS_CONSTANT = 0.0f;

    // Camera attributes
    const glm::vec3 JE_WORLD_UP = glm::vec3(0.0f, 1.0f, 0.0f);
    constexpr float JE_SCENE_VIEW_NEAR_PLANE = 0.1f;
    constexpr float JE_SCENE_VIEW_FAR_PLANE = 100.0f;
    constexpr float JE_SHADOW_VIEW_NEAR_PLANE = 0.1f;
    constexpr float JE_SHADOW_VIEW_FAR_PLANE = 100.0f;
    const float JE_FOVY = glm::radians(22.5f);

    // Vulkan Functions
    VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
    void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, const JEVulkanQueue& graphicsQueue, VkCommandPool commandPool);
    bool HasStencilComponent(VkFormat format);
    uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);

    using JECallbackFunction = std::function<void()>;

    // Various project file paths
    extern const std::string JE_PROJECT_PATH;
    extern const std::string JE_SHADER_DIR;
    extern const std::string JE_MODELS_OBJ_DIR;
    extern const std::string JE_TEXTURES_DIR;

    // Post Processing Shaders
    // The Joe Engine provides some built-in post processing shaders, distinguished by index in this array.
    extern const std::string JEBuiltInPostProcessingShaderPaths[2];

    // IO - defines for keys
    const int JE_KEY_W = GLFW_KEY_W;
    const int JE_KEY_A = GLFW_KEY_A;
    const int JE_KEY_S = GLFW_KEY_S;
    const int JE_KEY_D = GLFW_KEY_D;
    const int JE_KEY_Q = GLFW_KEY_Q;
    const int JE_KEY_E = GLFW_KEY_E;
    const int JE_KEY_UP = GLFW_KEY_UP;
    const int JE_KEY_LEFT = GLFW_KEY_LEFT;
    const int JE_KEY_DOWN = GLFW_KEY_DOWN;
    const int JE_KEY_RIGHT = GLFW_KEY_RIGHT;
    const int JE_KEY_0 = GLFW_KEY_0;
    const int JE_KEY_1 = GLFW_KEY_1;
    const int JE_KEY_2 = GLFW_KEY_2;
    const int JE_KEY_3 = GLFW_KEY_3;
    const int JE_KEY_4 = GLFW_KEY_4;
    const int JE_KEY_5 = GLFW_KEY_5;
    const int JE_KEY_6 = GLFW_KEY_6;
    const int JE_KEY_7 = GLFW_KEY_7;
    const int JE_KEY_8 = GLFW_KEY_8;
    const int JE_KEY_9 = GLFW_KEY_9;
}
