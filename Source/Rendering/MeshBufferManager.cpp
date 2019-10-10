#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "MeshBufferManager.h"

namespace JoeEngine {
    JEMesh_SSTriangle JEMeshBufferManager::m_screenSpaceTriangle{};

    void JEMeshBufferManager::Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue) {
        this->physicalDevice = physicalDevice;
        this->device = device;
        this->commandPool = commandPool;
        this->graphicsQueue = graphicsQueue;
        
        // Setup screen space triangle
        const std::vector<JEMeshVertex> screenSpaceTriangleVertices = { { glm::vec3(-1.0, -1.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 0.0) },
                                                                      { glm::vec3(3.0, -1.0, 0.0),  glm::vec3(0.0, 0.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(2.0, 0.0) },
                                                                      { glm::vec3(-1.0, 3.0, 0.0),  glm::vec3(0.0, 0.0, 0.0),  glm::vec3(0.0, 0.0, 0.0), glm::vec2(0.0, 2.0) } };
        const std::vector<uint32_t> screenSpaceTriangleIndices = { 2, 1, 0 };
        m_screenSpaceTriangle.vertexList = screenSpaceTriangleVertices;
        m_screenSpaceTriangle.indexList = screenSpaceTriangleIndices;
        CreateVertexBuffer(screenSpaceTriangleVertices, &m_screenSpaceTriangle.vertexBuffer, &m_screenSpaceTriangle.vertexBufferMemory);
        CreateIndexBuffer(screenSpaceTriangleIndices, &m_screenSpaceTriangle.indexBuffer, &m_screenSpaceTriangle.indexBufferMemory);
    }

    void JEMeshBufferManager::Cleanup() {
        for (uint32_t i = 0; i < m_numBuffers; ++i) {
            vkDestroyBuffer(device, m_vertexBuffers[i], nullptr);
            vkFreeMemory(device, m_vertexBufferMemory[i], nullptr);
            vkDestroyBuffer(device, m_indexBuffers[i], nullptr);
            vkFreeMemory(device, m_indexBufferMemory[i], nullptr);
        }
        vkDestroyBuffer(device, m_screenSpaceTriangle.vertexBuffer, nullptr);
        vkFreeMemory(device, m_screenSpaceTriangle.vertexBufferMemory, nullptr);
        vkDestroyBuffer(device, m_screenSpaceTriangle.indexBuffer, nullptr);
        vkFreeMemory(device, m_screenSpaceTriangle.indexBufferMemory, nullptr);
        m_numBuffers = 0;
    }

    MeshComponent JEMeshBufferManager::CreateMeshComponent(const std::string& filepath) {
        m_vertexBuffers.push_back(VK_NULL_HANDLE);
        m_indexBuffers.push_back(VK_NULL_HANDLE);
        m_vertexBufferMemory.push_back(VK_NULL_HANDLE);
        m_indexBufferMemory.push_back(VK_NULL_HANDLE);
        m_vertexLists.push_back(std::vector<JEMeshVertex>());
        m_indexLists.push_back(std::vector<uint32_t>());
        m_boundingBoxes.push_back(BoundingBoxData());
        LoadModelFromFile(filepath);
        CreateVertexBuffer(m_vertexLists[m_numBuffers], &m_vertexBuffers[m_numBuffers], &m_vertexBufferMemory[m_numBuffers]);
        CreateIndexBuffer(m_indexLists[m_numBuffers], &m_indexBuffers[m_numBuffers], &m_indexBufferMemory[m_numBuffers]);
        return MeshComponent((int)(m_numBuffers++));
    }

    void JEMeshBufferManager::LoadModelFromFile(const std::string& filepath) {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, filepath.c_str())) {
            throw std::runtime_error(err);
        }

        std::unordered_map<JEMeshVertex, uint32_t> uniqueVertices = {};
        std::vector<JEMeshVertex>& vertexList = m_vertexLists[m_numBuffers];

        // To compute the mesh's OBB (needed for physics), keep track of the min/max extents
        glm::vec3 minPos = glm::vec3(FLT_MAX);
        glm::vec3 maxPos = glm::vec3(-FLT_MAX);

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
                m_indexLists[m_numBuffers].push_back(uniqueVertices[vertex]);
            }
        }

        if (vertexList.size() > 0) {
            m_boundingBoxes[m_numBuffers][0] = minPos;
            m_boundingBoxes[m_numBuffers][1] = glm::vec3(minPos.x, minPos.y, maxPos.z);
            m_boundingBoxes[m_numBuffers][2] = glm::vec3(minPos.x, maxPos.y, minPos.z);
            m_boundingBoxes[m_numBuffers][3] = glm::vec3(minPos.x, maxPos.y, maxPos.z);
            m_boundingBoxes[m_numBuffers][4] = glm::vec3(maxPos.x, minPos.y, minPos.z);
            m_boundingBoxes[m_numBuffers][5] = glm::vec3(maxPos.x, minPos.y, maxPos.z);
            m_boundingBoxes[m_numBuffers][6] = glm::vec3(maxPos.x, maxPos.y, minPos.z);
            m_boundingBoxes[m_numBuffers][7] = maxPos;
        }
    }

    void JEMeshBufferManager::CreateVertexBuffer(const std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory) {
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

    void JEMeshBufferManager::CreateIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory) {
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
}
