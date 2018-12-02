#pragma once

#include <vector>
#include <array>

#include "../utils/Common.h"
#include "glm/gtx/hash.hpp"

struct MeshVertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 uv;

    MeshVertex() : MeshVertex(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0)) {}
    MeshVertex(glm::vec3 p, glm::vec3 c, glm::vec2 u) : pos(p), color(c), uv(u) {}

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




#define NUM_MESHES 100
// TODO: expand to freeze position x, y, z, etc, same for rotation
#define JE_PHYSICS_FREEZE_POSITION 1
#define JE_PHYSICS_FREEZE_ROTATION 2

// TODO: create a linked list of these
typedef struct mesh_data_graphics_t {
    VkBuffer vertexBufferArray[NUM_MESHES];
    VkDeviceMemory vertexBufferMemoryArray[NUM_MESHES];
    VkBuffer indexBufferArray[NUM_MESHES];
    VkDeviceMemory indexMemoryArray[NUM_MESHES];
    glm::mat4 modelMatrix[NUM_MESHES];
    std::vector<MeshVertex> vertexLists[NUM_MESHES];
    std::vector<uint32_t> indexLists[NUM_MESHES];
} MeshData_Graphics;

typedef struct mesh_data_physics_t {
    glm::vec3 positions[NUM_MESHES];
    glm::vec3 velocities[NUM_MESHES];
    glm::vec3 accelerations[NUM_MESHES];
    // TODO: rigidbody fields
    uint32_t freezeStates[NUM_MESHES];
} MeshData_Physics;

// Class for managing meshes that have graphics and physics needs in a data-oriented fashion.

class MeshDataManager {
private:
    MeshData_Graphics meshData_Graphics;
    MeshData_Physics meshData_Physics;
public:
    MeshDataManager() {}
    ~MeshDataManager() {}


};