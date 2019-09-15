#pragma once

namespace JoeEngine {
    class MeshComponent {
    private:
        /*
        vertbuffer handle
        index buffer handle
        */

        void Draw() const;
    public:
        friend class JEMeshComponentManager;
        friend class JEEngineInstance;

        MeshComponent() {}
        ~MeshComponent() {}
    };
}
