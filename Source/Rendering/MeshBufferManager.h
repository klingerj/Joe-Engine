#pragma once

#include <vector>
#include <array>

#include "vulkan/vulkan.h"
#include "glm/glm.hpp"
#include "glm/gtx/hash.hpp"

#include "../Utils/Common.h"
#include "VulkanRenderingTypes.h"
#include "VulkanQueue.h"
#include "../Components/Mesh/MeshComponent.h"   

namespace JoeEngine {
    class JEMeshBufferManager {
    private:
        std::vector<VkBuffer> m_vertexBuffers;
        std::vector<VkBuffer> m_indexBuffers;
        std::vector<VkDeviceMemory> m_vertexBufferMemory;
        std::vector<VkDeviceMemory> m_indexBufferMemory;
        std::vector<std::vector<JEMeshVertex>> m_vertexLists;
        std::vector<std::vector<uint32_t>> m_indexLists;
        uint16_t m_numBuffers; // TODO: make more intelligent w/ free list for when mesh data is no longer used

        // Vulkan object references (for convenience)
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkCommandPool commandPool;
        JEVulkanQueue graphicsQueue;

        // Mesh used for post processing
        static JEMesh_SSTriangle m_screenSpaceTriangle;

    public:
        JEMeshBufferManager() : m_numBuffers(0) {
            m_vertexBuffers.reserve(128);
            m_indexBuffers.reserve(128);
            m_vertexBufferMemory.reserve(128);
            m_indexBufferMemory.reserve(128);
            m_vertexLists.reserve(128);
            m_indexLists.reserve(128);
        }
        ~JEMeshBufferManager() {}

        void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue);
        void Cleanup();

        // TODO: overload to create from custom vert/idx buffer lists
        MeshComponent CreateMeshComponent(const std::string& filepath);
        void LoadModelFromFile(const std::string& filepath);
        void CreateVertexBuffer(const std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);

        // Getters
        const VkBuffer& GetVertexBufferAt(int index) {
            if (index < 0) {
                // TODO: throw?
            }

            return m_vertexBuffers[index];
        }

        const VkBuffer& GetIndexBufferAt(int index) {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexBuffers[index];
        }

        const std::vector<uint32_t>& GetIndexListAt(int index) {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexLists[index];
        }

        const JEMesh_SSTriangle& GetScreenSpaceTriMesh() const {
            return m_screenSpaceTriangle;
        }
    };
}

namespace std {
    template<> struct hash<JoeEngine::JEMeshVertex> {
        size_t operator()(JoeEngine::JEMeshVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal)) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}
