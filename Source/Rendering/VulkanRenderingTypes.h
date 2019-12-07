#pragma once

#include <stdint.h>
#include <array>

#include "vulkan/vulkan.h"

#include "../Utils/Common.h"

namespace JoeEngine {
    // Rendering-related structs

    typedef enum JE_PIPELINE_TYPE : uint8_t {
        FORWARD,
        DEFERRED,
        SHADOW,
        DEFERRED_GEOM,
        FORWARD_POINTS,
        TRANSLUCENT_OIT,
        TRANSLUCENT_OIT_SORT
    } PipelineType;

    // Order-independent translucency
    typedef struct oit_ll_node_t {
        glm::vec4 color; // color RBG, alpha
        glm::vec4 depth; // depth [0, 1], unused vec3
    } OITLinkedListNode;

    typedef struct oit_hp_node_t {
        uint32_t pointer;
    } OITHeadPointerNode;

    typedef struct oit_np_node_t {
        uint32_t pointer;
    } OITNextPointerNode;

    typedef struct oit_atomic_ctr_t {
        uint32_t atomicCtrData[4];
    } OITAtomicCounterData;

    // Generic Framebuffer attachment
    typedef struct je_framebuffer_attachment_t {
        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory deviceMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
    } JEFramebufferAttachment;

    typedef struct je_offscreen_forward_pass_t {
        uint32_t width = JE_DEFAULT_SCREEN_WIDTH, height = JE_DEFAULT_SCREEN_HEIGHT;
        VkFramebuffer framebuffer;
        JEFramebufferAttachment color;
        JEFramebufferAttachment depth;
        VkRenderPass renderPass;
        VkSampler sampler;
        VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
        VkSemaphore semaphore = VK_NULL_HANDLE; // Semaphore used to synchronize between this and the next render pass
    } JEForwardPass;

    // Render pass information for a shadow pass (depth-only)
    typedef struct je_offscreen_shadow_pass_t {
        uint32_t width = JE_DEFAULT_SHADOW_MAP_WIDTH, height = JE_DEFAULT_SHADOW_MAP_HEIGHT;
        std::vector<VkFramebuffer> framebuffers;
        std::vector<JEFramebufferAttachment> depths;
        VkRenderPass renderPass;
        VkSampler depthSampler;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> semaphores; // Semaphore used to synchronize between this and the next render pass
    } JEOffscreenShadowPass;

    // Render pass information for a deferred rendering pass (multiple g-buffers)
    typedef struct je_offscreen_deferred_pass_t {
        uint32_t width = JE_DEFAULT_SCREEN_WIDTH, height = JE_DEFAULT_SCREEN_HEIGHT;
        std::vector<VkFramebuffer> framebuffers;
        std::vector<JEFramebufferAttachment> colors;
        std::vector<JEFramebufferAttachment> normals;
        std::vector<JEFramebufferAttachment> depths;
        VkRenderPass renderPass;
        VkSampler sampler;
        std::vector<VkCommandBuffer> commandBuffers;
        std::vector<VkSemaphore> semaphores; // Semaphore used to synchronize between this and the next render pass
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

    struct JEMeshPointVertex {
        glm::vec3 pos;

        JEMeshPointVertex() : JEMeshPointVertex(glm::vec3(0.0f, 0.0f, 0.0f)) {}
        JEMeshPointVertex(glm::vec3 p) : pos(p) {}

        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription = {};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(JEMeshPointVertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }
        static std::array<VkVertexInputAttributeDescription, 1> getAttributeDescriptions() {
            std::array<VkVertexInputAttributeDescription, 1> attributeDescriptions = {};

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(JEMeshPointVertex, pos);

            return attributeDescriptions;
        }

        bool operator==(const JEMeshPointVertex& other) const {
            return pos == other.pos;
        }
    };

    typedef struct je_mesh_sstri_t {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        std::vector<JEMeshVertex> vertexList;
        std::vector<uint32_t> indexList;
    } JESingleMesh;
}
