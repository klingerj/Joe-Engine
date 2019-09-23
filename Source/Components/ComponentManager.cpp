#include "ComponentManager.h"

namespace JoeEngine {
    void JEMeshComponentManager::Update() {
        for (MeshComponent m : m_meshComponents) {
            m.Draw();
        }
    }

    MeshComponent JEMeshComponentManager::GetComponent(uint32_t idx) const {
        if (idx < 0 || idx > m_meshComponents.size()) {
            //TODO: throw exception or something
        }
        return m_meshComponents[idx];
    }

    const std::vector<MeshComponent>& JEMeshComponentManager::GetComponentList() const {
        return m_meshComponents;
    }
}
