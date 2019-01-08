#include "MeshDataManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

JEMeshData_SSTriangle JEMeshDataManager::screenSpaceTriangle {};

void JEMeshDataManager::Cleanup(VkDevice device) {
    for (uint32_t i = 0; i < numMeshes; ++i) {
        vkDestroyBuffer(device, meshData_Graphics.vertexBufferArray[i], nullptr);
        vkFreeMemory(device, meshData_Graphics.vertexBufferMemoryArray[i], nullptr);
        vkDestroyBuffer(device, meshData_Graphics.indexBufferArray[i], nullptr);
        vkFreeMemory(device, meshData_Graphics.indexBufferMemoryArray[i], nullptr);
    }
    vkDestroyBuffer(device, screenSpaceTriangle.vertexBuffer, nullptr);
    vkFreeMemory(device, screenSpaceTriangle.vertexBufferMemory, nullptr);
    vkDestroyBuffer(device, screenSpaceTriangle.indexBuffer, nullptr);
    vkFreeMemory(device, screenSpaceTriangle.indexBufferMemory, nullptr);
    numMeshes = 0;
    ResetState();
}

void JEMeshDataManager::ResetState() {
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Graphics.modelMatrices[i] = glm::mat4(1.0f); // Initialize model matrix to identity matrix
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Graphics.vertexLists[i] = std::vector<JEMeshVertex>();
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Graphics.indexLists[i] = std::vector<uint32_t>();
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.positions[i] = glm::vec3(0.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.velocities[i] = glm::vec3(0.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.accelerations[i] = glm::vec3(0.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.angularMomentums[i] = glm::vec3(0.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.rotations[i] = glm::mat3(1.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.scales[i] = glm::vec3(1.0f);
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.masses[i] = 1.0f;
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.obbs[i] = { { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) },
                                       glm::vec3(1.0f, 1.0f, 1.0f),
                                       glm::vec3(0.0f, 0.0f, 0.0f) };
    }
    for (uint32_t i = 0; i < JE_MAX_MESHES; ++i) {
        meshData_Physics.freezeStates[i] = JE_PHYSICS_FREEZE_POSITION | JE_PHYSICS_FREEZE_ROTATION;
    }
    screenSpaceTriangle.vertexList = std::vector<JEMeshVertex>();
    screenSpaceTriangle.indexList = std::vector<uint32_t>();
}

void JEMeshDataManager::CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::string& filepath, int freezeState) {
    LoadModelFromFile(filepath);
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, meshData_Graphics.vertexLists[numMeshes], &meshData_Graphics.vertexBufferArray[numMeshes], &meshData_Graphics.vertexBufferMemoryArray[numMeshes]);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, meshData_Graphics.indexLists[numMeshes], &meshData_Graphics.indexBufferArray[numMeshes], &meshData_Graphics.indexBufferMemoryArray[numMeshes]);
    meshData_Physics.freezeStates[numMeshes] = freezeState;
    ++numMeshes;
}

void JEMeshDataManager::CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices, int freezeState) {
    meshData_Graphics.vertexLists[numMeshes] = std::vector<JEMeshVertex>(vertices);
    meshData_Graphics.indexLists[numMeshes] = std::vector<uint32_t>(indices);
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, vertices, &meshData_Graphics.vertexBufferArray[numMeshes], &meshData_Graphics.vertexBufferMemoryArray[numMeshes]);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, indices, &meshData_Graphics.indexBufferArray[numMeshes], &meshData_Graphics.indexBufferMemoryArray[numMeshes]);
    meshData_Physics.freezeStates[numMeshes] = freezeState;
    ++numMeshes;
}

void JEMeshDataManager::CreateScreenSpaceTriangleMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue) {
    // Setup screen space triangle
    const std::vector<JEMeshVertex> screenSpaceTriangleVertices = { { glm::vec3(-1.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
                                                                  { glm::vec3(3.0, -1.0, 0.0),  glm::vec3(0.0, 0.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(2.0, 0.0) },
                                                                  { glm::vec3(-1.0, 3.0, 0.0),  glm::vec3(0.0, 0.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 2.0) } };
    const std::vector<uint32_t> screenSpaceTriangleIndices = { 2, 1, 0 };
    screenSpaceTriangle.vertexList = screenSpaceTriangleVertices;
    screenSpaceTriangle.indexList = screenSpaceTriangleIndices;
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, screenSpaceTriangleVertices, &screenSpaceTriangle.vertexBuffer, &screenSpaceTriangle.vertexBufferMemory);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, screenSpaceTriangleIndices, &screenSpaceTriangle.indexBuffer, &screenSpaceTriangle.indexBufferMemory);
}

void JEMeshDataManager::CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *vertexBuffer, *vertexBufferMemory);
    CopyBuffer(device, commandPool, graphicsQueue, stagingBuffer, *vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void JEMeshDataManager::CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue, const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory) {
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    CreateBuffer(physicalDevice, device, bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, *indexBuffer, *indexBufferMemory);

    CopyBuffer(device, commandPool, graphicsQueue, stagingBuffer, *indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void JEMeshDataManager::LoadModelFromFile(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str())) {
        throw std::runtime_error(err);
    }

    std::unordered_map<JEMeshVertex, uint32_t> uniqueVertices = {};
    std::vector<JEMeshVertex>& vertexList = meshData_Graphics.vertexLists[numMeshes];

    // To compute the mesh's OBB (needed for physics), keep track of the min/max extents
    glm::vec3 minPos = glm::vec3(9999999999.0f);
    glm::vec3 maxPos = glm::vec3(-9999999999.0f);

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            JEMeshVertex vertex = {};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            vertex.normal = {
                attrib.normals[3 * index.normal_index + 0],
                attrib.normals[3 * index.normal_index + 1],
                attrib.normals[3 * index.normal_index + 2]
            };

            vertex.uv = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

            vertex.color = { 1.0f, 1.0f, 1.0f };

            // Vertex deduplication
            if (uniqueVertices.count(vertex) == 0) {
                uniqueVertices[vertex] = static_cast<uint32_t>(vertexList.size());
                vertexList.push_back(vertex);
                minPos = glm::vec3(std::min(minPos.x, vertex.pos.x), std::min(minPos.y, vertex.pos.y), std::min(minPos.z, vertex.pos.z));
                maxPos = glm::vec3(std::max(maxPos.x, vertex.pos.x), std::max(maxPos.y, vertex.pos.y), std::max(maxPos.z, vertex.pos.z));
            }
            meshData_Graphics.indexLists[numMeshes].push_back(uniqueVertices[vertex]);
        }
    }
    if (vertexList.size() == 0) {
        meshData_Physics.obbs[numMeshes].e = glm::vec3(0.5f * (maxPos - minPos));
    } else {
        meshData_Physics.obbs[numMeshes].e = glm::vec3(0.5f * (maxPos - minPos));
    }
}

void JEMeshDataManager::DrawMesh(VkCommandBuffer commandBuffer, uint32_t index) {
    VkBuffer vertexBuffers[] = { meshData_Graphics.vertexBufferArray[index] };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, meshData_Graphics.indexBufferArray[index], 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(meshData_Graphics.indexLists[index].size()), 1, 0, 0, 0);
}

void JEMeshDataManager::DrawScreenSpaceTriangle(VkCommandBuffer commandBuffer) {
    VkBuffer vertexBuffers[] = { screenSpaceTriangle.vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, screenSpaceTriangle.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(screenSpaceTriangle.indexList.size()), 1, 0, 0, 0);
}
