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
    using BoundingBoxData = std::array<glm::vec3, 8>;

    class JEMeshBufferManager {
    private:
        std::vector<VkBuffer> m_vertexBuffers;
        std::vector<VkBuffer> m_indexBuffers;
        std::vector<VkDeviceMemory> m_vertexBufferMemory;
        std::vector<VkDeviceMemory> m_indexBufferMemory;
        std::vector<std::vector<JEMeshVertex>> m_vertexLists;
        std::vector<std::vector<uint32_t>> m_indexLists;
        std::vector<BoundingBoxData> m_boundingBoxes;
        uint16_t m_numBuffers; // TODO: make more intelligent w/ free list for when mesh data is no longer used

        // Vulkan object references (for convenience)
        VkPhysicalDevice physicalDevice;
        VkDevice device;
        VkCommandPool commandPool;
        JEVulkanQueue graphicsQueue;

        // Mesh used for post processing
        static JESingleMesh m_screenSpaceTriangle;
        static JESingleMesh m_boundingBoxMesh;

        void LoadModelFromFile(const std::string& filepath);
        void CreateVertexBuffer(const std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
        void CreateIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);

        void ComputeMeshBounds(const std::vector<JEMeshVertex>& vertices, uint32_t bufferId);

    public:
        JEMeshBufferManager() : m_numBuffers(0) {
            m_vertexBuffers.reserve(128);
            m_indexBuffers.reserve(128);
            m_vertexBufferMemory.reserve(128);
            m_indexBufferMemory.reserve(128);
            m_vertexLists.reserve(128);
            m_indexLists.reserve(128);
            m_boundingBoxes.reserve(128);
        }
        ~JEMeshBufferManager() {}

        void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue);
        void Cleanup();
        void ExpandMemberLists();

        MeshComponent CreateMeshComponent(const std::string& filepath);
        MeshComponent CreateMeshComponent(const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices);
        void UpdateMeshBuffer(uint32_t bufferId, const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices);

        // Getters
        const VkBuffer& GetVertexBufferAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_vertexBuffers[index];
        }

        const VkBuffer& GetIndexBufferAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexBuffers[index];
        }

        const std::vector<uint32_t>& GetIndexListAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexLists[index];
        }

        const std::vector<BoundingBoxData>& GetBoundingBoxData() const {
            return m_boundingBoxes;
        }

        const JESingleMesh& GetScreenSpaceTriMesh() const {
            return m_screenSpaceTriangle;
        }

        const JESingleMesh& GetBoundingBoxMesh() const {
            return m_boundingBoxMesh;
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
