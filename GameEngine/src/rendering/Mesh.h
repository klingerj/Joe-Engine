#pragma once

#include <string>
#include <vector>
#include <array>

#include "vulkan\vulkan.h"
#include "glm\glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "VulkanQueue.h"

struct MeshVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;

    // Necessary Vulkan functions
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MeshVertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(MeshVertex, uv);

        return attributeDescriptions;
    }

    bool operator==(const MeshVertex& other) const {
        return pos == other.pos && color == other.color && uv == other.uv;
    }
};

namespace std {
    template<> struct hash<MeshVertex> {
        size_t operator()(MeshVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}

// Class that contains mesh information such as vertices and loads from files

class Mesh {
private:
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<MeshVertex> vertices;
    std::vector<uint32_t> indices;

    void CreateBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
public:
    Mesh() {}
    ~Mesh() {}

    void Cleanup(const VkDevice& device);

    // Creation
    void LoadModelFromFile(const std::string& filepath);
    void CreateVertexBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkCommandPool commandPool, const VulkanQueue& graphicsQueue);
    void CreateIndexBuffer(const VkDevice& device, const VkPhysicalDevice& physDevice, VkCommandPool commandPool, const VulkanQueue& graphicsQueue);
    void Draw(const VkCommandBuffer& commandBuffer);
};
