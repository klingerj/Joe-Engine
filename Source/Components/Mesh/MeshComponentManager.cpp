#include "MeshComponentManager.h"

namespace JoeEngine {
    void JEMeshComponentManager::Update() {
        /*for (MeshComponent m : m_meshComponents) {
            // TODO
        }*/
    }

    void JEMeshComponentManager::AddNewComponent() {
        m_meshComponents.emplace_back(MeshComponent());
    }

    MeshComponent JEMeshComponentManager::GetComponent(uint32_t index) const {
        if (index > m_meshComponents.size() - 1) {
            //TODO: throw exception or something
        }
        return m_meshComponents[index];
    }

    void JEMeshComponentManager::SetComponent(uint32_t index, MeshComponent meshComp) {
        if (index > m_meshComponents.size() - 1) {
            //TODO: throw exception or something
        }

        m_meshComponents[index] = meshComp;
    }

    const std::vector<MeshComponent>& JEMeshComponentManager::GetComponentList() const {
        return m_meshComponents;
    }
}
