#pragma once

#include <string>
#include <vector>
#include <array>

#include "vulkan\vulkan.h"
#include "glm\glm.hpp"

#include "VulkanQueue.h"

struct MeshVertex {
    glm::vec3 pos;
    glm::vec3 color;

    // Necessary Vulkan functions
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MeshVertex, color);

        return attributeDescriptions;
    }
};

// Class that contains mesh information such as vertices and loads from files

class Mesh {
private:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<MeshVertex> vertices;
    std::vector<uint16_t> indices;
public:
    Mesh() {
        vertices = { { { -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
                     { { -0.5f, 0.5f, 0.0f },  { 0.0f, 1.0f, 0.0f } },
                     { { 0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
                     { { 0.5f, 0.5f, 0.0f },{ 1.0f, 1.0f, 1.0f } } };
        indices = { 2, 1, 0, 2, 3, 1 };
    }
    ~Mesh() {}

    void Cleanup(const VkDevice& device);

    // Creation
    void LoadModelFromFile(const std::string& filepath);
    void CreateBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void CreateVertexBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkCommandPool commandPool, const VulkanQueue& graphicsQueue);
    void CreateIndexBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkCommandPool commandPool, const VulkanQueue& graphicsQueue);
    void Draw(const VkCommandBuffer& commandBuffer);
};
