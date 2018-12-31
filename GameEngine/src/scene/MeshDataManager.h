#pragma once

#include <vector>
#include <array>

#include "../utils/Common.h"
#include "glm/gtx/hash.hpp"

struct MeshVertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;

    MeshVertex() : MeshVertex(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec2(0.0f, 0.0f)) {}
    MeshVertex(glm::vec3 p, glm::vec3 n, glm::vec3 c, glm::vec2 u) : pos(p), color(c), normal(n), uv(u) {}

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }
    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions = {};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(MeshVertex, normal);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(MeshVertex, color);

        attributeDescriptions[3].binding = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = offsetof(MeshVertex, uv);

        return attributeDescriptions;
    }

    bool operator==(const MeshVertex& other) const {
        return pos == other.pos && normal == other.normal && color == other.color && uv == other.uv;
    }
};

namespace std {
    template<> struct hash<MeshVertex> {
        size_t operator()(MeshVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal)) ^
                    (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                    (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}

#define MAX_MESHES 100
// TODO: expand to freeze position x, y, z, etc, same for rotation
// Also TODO: change this to an enum and update the MeshDataManager's CreateNewMesh function parameters from int to the enum
#define JE_PHYSICS_FREEZE_NONE 0
#define JE_PHYSICS_FREEZE_POSITION 1
#define JE_PHYSICS_FREEZE_ROTATION 2

// TODO: create a linked list of these
typedef struct mesh_data_graphics_t {
    VkBuffer vertexBufferArray[MAX_MESHES];
    VkDeviceMemory vertexBufferMemoryArray[MAX_MESHES];
    VkBuffer indexBufferArray[MAX_MESHES];
    VkDeviceMemory indexBufferMemoryArray[MAX_MESHES];
    glm::mat4 modelMatrices[MAX_MESHES];
    std::vector<MeshVertex> vertexLists[MAX_MESHES];
    std::vector<uint32_t> indexLists[MAX_MESHES];
} MeshData_Graphics;

typedef struct oriented_bounding_box_t {
    glm::vec3 u[3]; // Local OBB axes
    glm::vec3 e; // extents of local OBB axes (halfwidths)
    glm::vec3 center; // position of the OBB
} OBB;

typedef struct mesh_data_physics_t {
    glm::vec3 positions[MAX_MESHES];
    glm::vec3 velocities[MAX_MESHES];
    glm::vec3 accelerations[MAX_MESHES];
    glm::vec3 angularMomentums[MAX_MESHES];
    glm::mat3 rotations[MAX_MESHES]; // TODO: change to quaternion
    glm::vec3 scales[MAX_MESHES];
    float masses[MAX_MESHES];
    OBB obbs[MAX_MESHES];
    uint32_t freezeStates[MAX_MESHES];
} MeshData_Physics;

typedef struct mesh_data_sstri_t {
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<MeshVertex> vertexList;
    std::vector<uint32_t> indexList;
} MeshData_SSTriangle;

// Class for managing meshes that have graphics and physics needs in a data-oriented fashion.

class MeshDataManager {
private:
    MeshData_Graphics meshData_Graphics;
    MeshData_Physics meshData_Physics;
    uint32_t numMeshes; // # of meshes added so far
    // ScreenSpace Triangle - needed for deferred rendering lighting pass and post processing. Store separately from other mesh data.
    static MeshData_SSTriangle screenSpaceTriangle;

    // Mesh Creation
    void CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<MeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);
    void CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);
    void LoadModelFromFile(const std::string& filepath);

public:
    MeshDataManager() : numMeshes(0) {
        ResetState();
    }
    ~MeshDataManager() {}
    
    void Cleanup(VkDevice device);
    void ResetState();

    // TODO: add checks for bad indices
    void SetModelMatrix(const glm::mat4& m, uint32_t index) {
        meshData_Graphics.modelMatrices[index] = m;
    }
    void SetMeshPosition(const glm::vec3& pos, uint32_t index) {
        meshData_Physics.positions[index] = pos;
        glm::mat4& modelMat = meshData_Graphics.modelMatrices[index];
        modelMat[3] = glm::vec4(pos, 1.0f);
    }
    void SetMeshScale(const glm::vec3& scale, uint32_t index) { //TODO: connect to model matrix
        meshData_Physics.scales[index] = scale;
    }

    void CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::string& filepath, int freezeState);

    // TODO: This is not needed, replace with a function that creates a duplicate of an existing mesh. Later, this will need an offset buffer for instanced rendering.
    void CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, int freezeState);
    void CreateScreenSpaceTriangleMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue);
    void DrawMesh(VkCommandBuffer commandBuffer, uint32_t index);
    void DrawScreenSpaceTriangle(VkCommandBuffer commandBuffer);

    // Getters
    const glm::mat4& GetModelMatrix(uint32_t index) const {
        return meshData_Graphics.modelMatrices[index];
    }
    uint32_t GetNumMeshes() const {
        return numMeshes;
    }
    MeshData_Physics& GetMeshData_Physics() {
        return meshData_Physics;
    }
    const glm::mat4* GetModelMatrices() const {
        return meshData_Graphics.modelMatrices;
    }
};
