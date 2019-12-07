#pragma once

namespace JoeEngine {
    enum JE_MESH_BUFFER_TYPE {
        MESH_TRIANGLES,
        MESH_LINES,
        MESH_POINTS
    };

    class MeshComponent {
    private:
        int m_vertexBufferHandle;
        int m_indexBufferHandle;
        JE_MESH_BUFFER_TYPE m_type;

    public:
        MeshComponent() : MeshComponent(-1, MESH_TRIANGLES) {}
        MeshComponent(int h, JE_MESH_BUFFER_TYPE t) : m_vertexBufferHandle(h), m_indexBufferHandle(h), m_type(t) {}
        MeshComponent(int v, int i, JE_MESH_BUFFER_TYPE t) : m_vertexBufferHandle(v), m_indexBufferHandle(i), m_type(t) {}
        ~MeshComponent() {}

        int GetVertexHandle() const {
            return m_vertexBufferHandle;
        }

        int GetIndexHandle() const {
            return m_indexBufferHandle;
        }
    };
}
