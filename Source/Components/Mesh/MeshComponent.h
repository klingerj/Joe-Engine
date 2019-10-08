#pragma once

namespace JoeEngine {
    //!  The Mesh Component class
    /*!
      Contains the necessary mesh info to be attached to a particular entity.
      This consists of a single vertex buffer handle and a single index buffer handle.
      These handles are used to access mesh data managed by th MeshBufferManager class.
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

    public:
        //! Default constructor.
        /*! Constructs components with invalid vertex/index buffer handles. */
        MeshComponent() : MeshComponent(-1, -1) {}

        //! Constructor.
        /*! Constructs component with specified vetex and index buffer handles. */
        MeshComponent(int vertexBufferHandle, int indexBufferHandle) : m_vertexBufferHandle(vertexBufferHandle),
                                                                       m_indexBufferHandle(indexBufferHandle) {}

        //! Destructor (default).
        ~MeshComponent() = default;

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
