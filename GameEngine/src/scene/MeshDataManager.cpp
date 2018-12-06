#include "MeshDataManager.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <iostream>

MeshData_SSTriangle MeshDataManager::screenSpaceTriangle {};

void MeshDataManager::Cleanup(VkDevice device) {
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
}

void MeshDataManager::CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::string& filepath, int freezeState) {
    LoadModelFromFile(filepath);
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, meshData_Graphics.vertexLists[numMeshes], &meshData_Graphics.vertexBufferArray[numMeshes], &meshData_Graphics.vertexBufferMemoryArray[numMeshes]);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, meshData_Graphics.indexLists[numMeshes], &meshData_Graphics.indexBufferArray[numMeshes], &meshData_Graphics.indexBufferMemoryArray[numMeshes]);
    meshData_Physics.freezeStates[numMeshes] = freezeState;
    ++numMeshes;
}

void MeshDataManager::CreateNewMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices, int freezeState) {
    meshData_Graphics.vertexLists[numMeshes] = std::vector<MeshVertex>(vertices);
    meshData_Graphics.indexLists[numMeshes] = std::vector<uint32_t>(indices);
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, vertices, &meshData_Graphics.vertexBufferArray[numMeshes], &meshData_Graphics.vertexBufferMemoryArray[numMeshes]);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, indices, &meshData_Graphics.indexBufferArray[numMeshes], &meshData_Graphics.indexBufferMemoryArray[numMeshes]);
    meshData_Physics.freezeStates[numMeshes] = freezeState;
    ++numMeshes;
}

void MeshDataManager::CreateScreenSpaceTriangleMesh(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue) {
    // Setup screen space triangle
    const std::vector<MeshVertex> screenSpaceTriangleVertices = { { glm::vec3(-1.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
                                                                  { glm::vec3(3.0, -1.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(2.0, 0.0) },
                                                                  { glm::vec3(-1.0, 3.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 2.0) } };
    const std::vector<uint32_t> screenSpaceTriangleIndices = { 2, 1, 0 };
    screenSpaceTriangle.vertexList = screenSpaceTriangleVertices;
    screenSpaceTriangle.indexList = screenSpaceTriangleIndices;
    CreateVertexBuffer(physicalDevice, device, commandPool, graphicsQueue, screenSpaceTriangleVertices, &screenSpaceTriangle.vertexBuffer, &screenSpaceTriangle.vertexBufferMemory);
    CreateIndexBuffer(physicalDevice, device, commandPool, graphicsQueue, screenSpaceTriangleIndices, &screenSpaceTriangle.indexBuffer, &screenSpaceTriangle.indexBufferMemory);
}

void MeshDataManager::CreateVertexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<MeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory) {
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

void MeshDataManager::CreateIndexBuffer(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const VulkanQueue& graphicsQueue, const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory) {
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

void MeshDataManager::LoadModelFromFile(const std::string& filepath) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str())) {
        throw std::runtime_error(err);
    }

    std::unordered_map<MeshVertex, uint32_t> uniqueVertices = {};
    std::vector<MeshVertex>& vertexList = meshData_Graphics.vertexLists[numMeshes];

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            MeshVertex vertex = {};

            vertex.pos = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
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
            }
            meshData_Graphics.indexLists[numMeshes].push_back(uniqueVertices[vertex]);
        }
    }
    std::cout << "Number of vertices loaded: " << vertexList.size() << std::endl;
    std::cout << "Number of indices loaded: " << meshData_Graphics.indexLists[numMeshes].size() << std::endl;
}

void MeshDataManager::DrawMesh(VkCommandBuffer commandBuffer, uint32_t index) {
    VkBuffer vertexBuffers[] = { meshData_Graphics.vertexBufferArray[index] };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, meshData_Graphics.indexBufferArray[index], 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(meshData_Graphics.indexLists[index].size()), 1, 0, 0, 0);
}

void MeshDataManager::DrawScreenSpaceTriangle(VkCommandBuffer commandBuffer) {
    VkBuffer vertexBuffers[] = { screenSpaceTriangle.vertexBuffer };
    VkDeviceSize offsets[] = { 0 };

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, screenSpaceTriangle.indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(screenSpaceTriangle.indexList.size()), 1, 0, 0, 0);
}
