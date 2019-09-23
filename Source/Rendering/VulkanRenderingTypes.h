#pragma once

#include <stdint.h>
#include <array>

#include "vulkan/vulkan.h"

namespace JoeEngine {
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

    // Render pass information for a post processing pass
    typedef struct je_post_processing_pass_t {
        uint32_t width = JE_DEFAULT_SCREEN_WIDTH, height = JE_DEFAULT_SCREEN_HEIGHT;
        VkFramebuffer framebuffer = VK_NULL_HANDLE;
        JEFramebufferAttachment texture;
        VkRenderPass renderPass;
        VkSampler sampler;
        uint32_t shaderIndex = -1; // ID indicating which built-in post shader to use. -1 for custom shader.
        std::string filepath = ""; // Path to custom shader if not using a built-in.
    } JEPostProcessingPass;

    struct JEMeshVertex {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;

        JEMeshVertex() : JEMeshVertex(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)) {}
        JEMeshVertex(glm::vec3 p, glm::vec3 n, glm::vec3 c, glm::vec2 u) : pos(p), color(c), normal(n), uv(u) {}

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(JEMeshVertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }
        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(JEMeshVertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(JEMeshVertex, normal);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(JEMeshVertex, color);

            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(JEMeshVertex, uv);

            return attributeDescriptions;
        }

        bool operator==(const JEMeshVertex& other) const {
            return pos == other.pos && normal == other.normal && color == other.color && uv == other.uv;
        }
    };

    typedef struct je_mesh_sstri_t {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        std::vector<JEMeshVertex> vertexList;
        std::vector<uint32_t> indexList;
    } JEMesh_SSTriangle;
}
