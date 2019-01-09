#pragma once

#include <vector>
#include <array>

#include "../utils/Common.h"
#include "glm/gtx/hash.hpp"

namespace JoeEngine {
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
        static ::std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
            ::std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

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

    #define JE_MAX_MESHES 100
    // TODO: expand to freeze position x, y, z, etc, same for rotation
    // Also TODO: change this to an enum and update the MeshDataManager's CreateNewMesh function parameters from int to the enum
    #define JE_PHYSICS_FREEZE_NONE 0
    #define JE_PHYSICS_FREEZE_POSITION 1
    #define JE_PHYSICS_FREEZE_ROTATION 2

    typedef struct je_mesh_data_graphics_t {
        VkBuffer vertexBufferArray[JE_MAX_MESHES];
        VkDeviceMemory vertexBufferMemoryArray[JE_MAX_MESHES];
        VkBuffer indexBufferArray[JE_MAX_MESHES];
        VkDeviceMemory indexBufferMemoryArray[JE_MAX_MESHES];
        glm::mat4 modelMatrices[JE_MAX_MESHES];
        ::std::vector<JEMeshVertex> vertexLists[JE_MAX_MESHES];
        ::std::vector<uint32_t> indexLists[JE_MAX_MESHES];
    } JEMeshData_Graphics;

    typedef struct je_oriented_bounding_box_t {
        glm::vec3 u[3]; // Local OBB axes
        glm::vec3 e; // extents of local OBB axes (halfwidths)
        glm::vec3 center; // position of the OBB
    } JE_OBB;

    typedef struct je_mesh_data_physics_t {
        glm::vec3 positions[JE_MAX_MESHES];
        glm::vec3 velocities[JE_MAX_MESHES];
        glm::vec3 accelerations[JE_MAX_MESHES];
        glm::vec3 angularMomentums[JE_MAX_MESHES];
        glm::mat3 rotations[JE_MAX_MESHES]; // TODO: change to quaternion
        glm::vec3 scales[JE_MAX_MESHES];
        float masses[JE_MAX_MESHES];
        JE_OBB obbs[JE_MAX_MESHES];
        uint32_t freezeStates[JE_MAX_MESHES];
    } JEMeshData_Physics;

    typedef struct je_mesh_data_sstri_t {
        VkBuffer vertexBuffer;
        VkDeviceMemory vertexBufferMemory;
        VkBuffer indexBuffer;
        VkDeviceMemory indexBufferMemory;
        ::std::vector<JEMeshVertex> vertexList;
        ::std::vector<uint32_t> indexList;
    } JEMeshData_SSTriangle;

    // Class for managing meshes that have graphics and physics needs in a data-oriented fashion.

    class JEMeshDataManager {
    private:
        JEMeshData_Graphics m_meshData_Graphics;
        JEMeshData_Physics m_meshData_Physics;
        uint32_t m_numMeshes; // # of meshes added so far
        // ScreenSpace Triangle - needed for deferred rendering lighting pass and post processing. Store separately from other mesh data.
        static JEMeshData_SSTriangle m_screenSpaceTriangle;

        // Mesh Creation
        void CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const ::std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
        void CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const ::std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);
        void LoadModelFromFile(const ::std::string& filepath);

    public:
        JEMeshDataManager() : m_numMeshes(0) {
            ResetState();
        }
        ~JEMeshDataManager() {}

        void Cleanup(VkDevice device);
        void ResetState();

        // TODO: add checks for bad indices
        void SetModelMatrix(const glm::mat4& m, uint32_t index) {
            m_meshData_Graphics.modelMatrices[index] = m;
        }
        void SetMeshPosition(const glm::vec3& pos, uint32_t index) {
            m_meshData_Physics.positions[index] = pos;
            glm::mat4& modelMat = m_meshData_Graphics.modelMatrices[index];
            modelMat[3] = glm::vec4(pos, 1.0f);
        }
        void SetMeshScale(const glm::vec3& scale, uint32_t index) { //TODO: connect to model matrix
            m_meshData_Physics.scales[index] = scale;
        }

        void CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const ::std::string& filepath, int freezeState);

        // TODO: This is not needed, replace with a function that creates a duplicate of an existing mesh. Later, this will need an offset buffer for instanced rendering.
        void CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const ::std::vector<JEMeshVertex>& vertices, const ::std::vector<uint32_t>& indices, int freezeState);
        void CreateScreenSpaceTriangleMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue);
        void DrawMesh(VkCommandBuffer commandBuffer, uint32_t index);
        void DrawScreenSpaceTriangle(VkCommandBuffer commandBuffer);

        // Getters
        const glm::mat4& GetModelMatrix(uint32_t index) const {
            return m_meshData_Graphics.modelMatrices[index];
        }
        uint32_t GetNumMeshes() const {
            return m_numMeshes;
        }
        JEMeshData_Physics& GetMeshData_Physics() {
            return m_meshData_Physics;
        }
        const glm::mat4* GetModelMatrices() const {
            return m_meshData_Graphics.modelMatrices;
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
