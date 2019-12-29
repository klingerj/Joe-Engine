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
    //! Typedef for bounding box data - a bounding box is a list of 8 3D points (corners of the box).
    using BoundingBoxData = std::array<glm::vec3, 8>;

    //! The JEMeshBufferManager
    /*!
      Class that manages all mesh buffer data, from loading to access.
      Provides convenience functions such as creating Mesh Components (for usage with the Joe Engine's entity-component system)
      and updating mesh buffers.
    */
    class JEMeshBufferManager {
    private:
        //! List of verex buffers.
        std::vector<VkBuffer> m_vertexBuffers;

        //! List of index buffers.
        std::vector<VkBuffer> m_indexBuffers;

        //! List of on-device vertex buffers.
        std::vector<VkDeviceMemory> m_vertexBufferMemory;

        //! List of on-device index buffers.
        std::vector<VkDeviceMemory> m_indexBufferMemory;

        //! List of JEMeshVertex data lists (each element is a triangle mesh; a list of JEMeshVertex's).
        std::vector<std::vector<JEMeshVertex>> m_vertexLists;

        //! List of JEMeshPointVertex data lists (each element is a point mesh; a list of JEMeshPointVertex's).
        std::vector<std::vector<JEMeshPointVertex>> m_vertexPointLists;

        //! List of mesh indices lists.
        std::vector<std::vector<uint32_t>> m_indexLists;

        //! List of mesh bounding boxes.
        std::vector<BoundingBoxData> m_boundingBoxes;

        //! Number of buffers currently being stored.
        uint16_t m_numBuffers; // TODO: make more intelligent w/ free list for when mesh data is no longer used

        //! Reference to Vulkan physical device.
        VkPhysicalDevice physicalDevice;

        //! Reference to Vulkan logical device.
        VkDevice device;

        //! Reference to Vulkan command pool.
        VkCommandPool commandPool;

        //! Reference to Vulkan graphics queue.
        JEVulkanQueue graphicsQueue;

        //! Screen space triangle mesh.
        //! Mesh used for post processing. (Only one instance of this mesh is necessary.)
        static JESingleMesh m_screenSpaceTriangle;

        //! Bounding box mesh.
        //! Mesh used for visualizing and entity's bounding box. (Only one instance of this mesh is necessary.)
        static JESingleMesh m_boundingBoxMesh;

        //! Loads a model from a file.
        /*!
          \param filepath the mesh file source path.
        */
        void LoadModelFromFile(const std::string& filepath);

        //! Creates a vertex buffer given a list of triangle mesh vertices.
        /*!
          \param vertices list of triangle mesh vertices.
          \param vertexBuffer the Vulkan buffer to copy the vertex data to.
          \param vertexBufferMemory the Vulkan device memory buffer to copy the vertex data to.
        */
        void CreateVertexBuffer(const std::vector<JEMeshVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);

        //! Creates a vertex buffer given a list of point mesh vertices.
        /*!
          \param vertices list of point mesh vertices.
          \param vertexBuffer the Vulkan buffer to copy the vertex data to.
          \param vertexBufferMemory the Vulkan device memory buffer to copy the vertex data to.
        */
        void CreateVertexBuffer(const std::vector<JEMeshPointVertex>& vertices, VkBuffer* vertexBuffer, VkDeviceMemory* vertexBufferMemory);

        //! Creates an index buffer given a list of mesh vertices.
        /*!
          \param indices list of mesh indices.
          \param indexBuffer the Vulkan buffer to copy the index data to.
          \param indexBufferMemory the Vulkan device memory buffer to copy the index data to.
        */
        void CreateIndexBuffer(const std::vector<uint32_t>& indices, VkBuffer* indexBuffer, VkDeviceMemory* indexBufferMemory);

        //! Computes the bounding box data for a triangle mesh.
        /*!
          \param vertices the list of triangle mesh vertiex.
          \param bufferId the ID of the mesh buffer to compute bounding box data for.
        */
        void ComputeMeshBounds(const std::vector<JEMeshVertex>& vertices, uint32_t bufferId);
        
        //! Computes the bounding box data for a point mesh.
        /*!
          \param vertices the list of point mesh vertiex.
          \param bufferId the ID of the mesh buffer to compute bounding box data for.
        */
        void ComputeMeshBounds(const std::vector<JEMeshPointVertex>& vertices, uint32_t bufferId);

    public:
        //! Default constructor.
        //! Initializes member variables and reserve data for each member list.
        JEMeshBufferManager() : m_numBuffers(0) {
            m_vertexBuffers.reserve(128);
            m_indexBuffers.reserve(128);
            m_vertexBufferMemory.reserve(128);
            m_indexBufferMemory.reserve(128);
            m_vertexLists.reserve(128);
            m_indexLists.reserve(128);
            m_boundingBoxes.reserve(128);
        }

        //! Destructor (default).
        ~JEMeshBufferManager() = default;

        //! Initialization.
        /*!
          \param physicalDevice the Vulkan physical device.
          \param device the Vulkan logical device.
          \param commandPool the Vulkan command pool.
          \param graphicsQueue the Vulkan graphics queue.
        */
        void Initialize(VkPhysicalDevice physicalDevice, VkDevice device, VkCommandPool commandPool, const JEVulkanQueue& graphicsQueue);

        //! Cleanup all Vulkan objects and memory.
        void Cleanup();

        //! Preps member lists for new data.
        void ExpandMemberLists();

        //! Create a new Mesh Component.
        /*!
          \param filepath the mesh file source path.
          \return a new Mesh Component.
        */
        MeshComponent CreateMeshComponent(const std::string& filepath);

        //! Create a new Mesh Component from existing vertex and index lists.
        /*!
          \param vertices the existing list of triangle mesh vertices.
          \param indices the existing list of mesh indices.
        */
        MeshComponent CreateMeshComponent(const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices);
        
        //! Create a new Mesh Component from existing vertex and index lists.
        /*!
          \param vertices the existing list of point mesh vertices.
          \param indices the existing list of mesh indices.
        */
        MeshComponent CreateMeshComponent(const std::vector<JEMeshPointVertex>& vertices, const std::vector<uint32_t>& indices);

        //! Update a mesh buffer to a new list of vertices and indices.
        /*!
          \param bufferId the ID of the mesh buffer to update.
          \param vertices the new list of triangle mesh vertices.
          \param indices the new list of mesh indices.
        */
        void UpdateMeshBuffer(uint32_t bufferId, const std::vector<JEMeshVertex>& vertices, const std::vector<uint32_t>& indices);
        
        //! Update a mesh buffer to a new list of vertices and indices.
        /*!
          \param bufferId the ID of the mesh buffer to update.
          \param vertices the new list of point mesh vertices.
          \param indices the new list of mesh indices.
        */
        void UpdateMeshBuffer(uint32_t bufferId, const std::vector<JEMeshPointVertex>& vertices, const std::vector<uint32_t>& indices);

        //! Get vertex buffer at index.
        /*!
          Returns the vertex buffer corresponding to the given index / mesh buffer ID.
          \param index the mesh ID whose vertex buffer to return.
          \return the vertex buffer corresponding to the index.
        */
        const VkBuffer& GetVertexBufferAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_vertexBuffers[index];
        }

        //! Get index buffer at index.
        /*!
          Returns the index buffer corresponding to the given index / mesh buffer ID.
          \param index the mesh ID whose index buffer to return.
          \return the index buffer corresponding to the index.
        */
        const VkBuffer& GetIndexBufferAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexBuffers[index];
        }

        //! Get index list at index.
        /*!
          Returns the index list corresponding to the given index / mesh buffer ID.
          \param index the mesh ID whose index list to return.
          \return the index list corresponding to the index.
        */
        const std::vector<uint32_t>& GetIndexListAt(int index) const {
            if (index < 0) {
                // TODO: throw?
            }

            return m_indexLists[index];
        }

        //! Get all bounding box data.
        /*!
          Returns the member list of all bounding box data.
          \return const-reference to the list of all bounding box data.
        */
        const std::vector<BoundingBoxData>& GetBoundingBoxData() const {
            return m_boundingBoxes;
        }

        //! Get the single-instance screen-space triangle mesh struct data.
        const JESingleMesh& GetScreenSpaceTriMesh() const {
            return m_screenSpaceTriangle;
        }

        //! Get the single-instance bounding box mesh struct data.
        const JESingleMesh& GetBoundingBoxMesh() const {
            return m_boundingBoxMesh;
        }
    };
}

/*! \cond PRIVATE */
namespace std {
    template<> struct hash<JoeEngine::JEMeshVertex> {
        size_t operator()(JoeEngine::JEMeshVertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal)) ^
                (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.uv) << 1);
        }
    };
}
/*! \endcond */
