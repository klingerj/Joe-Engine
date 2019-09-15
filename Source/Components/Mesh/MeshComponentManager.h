#pragma once

#include <vector>

#include "MeshComponent.h"

namespace JoeEngine {
    class JEMeshComponentManager {
    private:
        std::vector<MeshComponent> m_meshComponents;
    public:
        JEMeshComponentManager() {
            m_meshComponents.reserve(128);
        }
        ~JEMeshComponentManager() {}

        JEMeshComponentManager(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager(JEMeshComponentManager&& mgr) = delete;
        JEMeshComponentManager& operator=(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager& operator=(JEMeshComponentManager&& mgr) = delete;

        void Update();
        MeshComponent GetComponent(uint32_t idx) const;
        const std::vector<MeshComponent>& GetComponentList() const;
    };
}
