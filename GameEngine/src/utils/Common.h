#pragma once

#include <string>

#include "vulkan/vulkan.h"
#include "../rendering/VulkanQueue.h"
#include "glm/glm.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

// Constants for rendering
#define DEFAULT_SCREEN_WIDTH 800
#define DEFAULT_SCREEN_HEIGHT 600
#define DEFAULT_MAX_FRAMES_IN_FLIGHT 2

// Camera attributes
#define WORLD_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.0f
#define FOVY glm::radians(45.0f)

// Various project file paths
extern std::string PROJECT_PATH;
extern std::string SHADER_DIR;
extern std::string MODELS_OBJ_DIR;
extern std::string TEXTURES_DIR;

// Vulkan Functions
VkCommandBuffer BeginSingleTimeCommands(VkDevice device, VkCommandPool commandPool);
void EndSingleTimeCommands(VkDevice device, VkCommandBuffer commandBuffer, const VulkanQueue& graphicsQueue, VkCommandPool commandPool);
uint32_t FindMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
void CreateBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
VkImageView CreateImageView(VkDevice device, VkImage image, VkFormat format);
