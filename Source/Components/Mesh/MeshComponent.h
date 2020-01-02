#pragma once

namespace JoeEngine {
    //! Mesh Buffer Type enum
    /*! Indicates what kind of buffer a mesh component refers to. */
    enum JE_MESH_BUFFER_TYPE {
        MESH_TRIANGLES,
        MESH_LINES,
        MESH_POINTS
    };

    //! The Mesh Component class
    /*!
      Contains the necessary mesh info to be attached to a particular entity.
      These handles are used to access mesh data managed by the MeshBufferManager class.
      \sa JEMeshComponentManager, JEMeshBufferManager, JEVulkanRenderer
    */
    class MeshComponent {
    private:
        //! Vertex buffer handle.
        /*! Handle to the intended vertex buffer to use with this mesh. */
        int m_vertexBufferHandle;

        //! Index buffer handle.
        /*! Handle to the intended index buffer to use with this mesh. */
        int m_indexBufferHandle;

        //! Mesh buffer type.
        /*! Indicates what kind of geometry the mesh indices is intended for. */
        JE_MESH_BUFFER_TYPE m_type;

    public:
        //! Default constructor.
        /*! Constructs components with invalid vertex/index buffer handles and triangle type. */
        MeshComponent() : MeshComponent(-1, MESH_TRIANGLES) {}

        //! Constructor.
        /*!
        Constructs component with specified vertex buffer handle and type.
        The index buffer will have the same value as the vertex buffer.
        */
        MeshComponent(int h, JE_MESH_BUFFER_TYPE t) : m_vertexBufferHandle(h), m_indexBufferHandle(h), m_type(t) {}

        //! Constructor.
        /*! Constructs component with specified vertex / index buffer handles and type. */
        MeshComponent(int v, int i, JE_MESH_BUFFER_TYPE t) : m_vertexBufferHandle(v), m_indexBufferHandle(i), m_type(t) {}

        //! Destructor (default).
        ~MeshComponent() {}

        //! Vertex handle Getter.
        /*! Returns the component's vertex buffer handle. */
        int GetVertexHandle() const {
            return m_vertexBufferHandle;
        }

        //! Index handle Getter.
        /*! Returns the component's index buffer handle. */
        int GetIndexHandle() const {
            return m_indexBufferHandle;
        }
    };
}
