#pragma once

namespace JoeEngine {
    class MeshComponent {
    private:
        int m_vertexBufferHandle;
        int m_indexBufferHandle;

    public:
        MeshComponent() : MeshComponent(-1) {}
        MeshComponent(int h) : m_vertexBufferHandle(h), m_indexBufferHandle(h) {}
        MeshComponent(int v, int i) : m_vertexBufferHandle(v), m_indexBufferHandle(i) {}
        ~MeshComponent() {}

        int GetVertexHandle() const {
            return m_vertexBufferHandle;
        }

        int GetIndexHandle() const {
            return m_indexBufferHandle;
        }
    };
}
