#pragma once

#include <string>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_LEFT_HANDED

#include "vulkan/vulkan.h"
#include "../rendering/VulkanQueue.h"
#include "glm/glm.hpp"

// Constants for rendering
constexpr int DEFAULT_SCREEN_WIDTH = 800;
constexpr int DEFAULT_SCREEN_HEIGHT = 600;
constexpr int DEFAULT_MAX_FRAMES_IN_FLIGHT = 2;

constexpr int DEFAULT_SHADOW_MAP_WIDTH = 2000;
constexpr int DEFAULT_SHADOW_MAP_HEIGHT = 2000;
constexpr float DEFAULT_SHADOW_MAP_DEPTH_BIAS_SLOPE = 1.75f;
constexpr float DEFAULT_SHADOW_MAP_DEPTH_BIAS_CONSTANT = 0.0f;

// Camera attributes
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
constexpr float SCENE_VIEW_NEAR_PLANE = 0.1f;
constexpr float SCENE_VIEW_FAR_PLANE = 100.0f;
constexpr float SHADOW_VIEW_NEAR_PLANE = 0.1f;
constexpr float SHADOW_VIEW_FAR_PLANE = 20.0f;
#define FOVY glm::radians(45.0f)

// Various project file paths
extern std::string PROJECT_PATH;
extern std::string SHADER_DIR;
extern std::string MODELS_OBJ_DIR;
extern std::string TEXTURES_DIR;

// Vulkan Functions
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, const VulkanQueue& graphicsQueue, VkCommandPool commandPool);
bool HasStencilComponent(VkFormat format);
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CreateImage(VkPhysicalDevice physicalDevice, VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
void TransitionImageLayout(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
VkFormat FindSupportedFormat(VkPhysicalDevice physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
VkFormat FindDepthFormat(VkPhysicalDevice physicalDevice);
