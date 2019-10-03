#include "MeshComponentManager.h"

namespace JoeEngine {
    void JEMeshComponentManager::Update() {
        /*for (MeshComponent m : m_meshComponents) {
            // TODO
        }*/
    }

    void JEMeshComponentManager::AddNewComponent(uint32_t id) {
        m_meshComponents.AddElement(id, MeshComponent());
    }

    void JEMeshComponentManager::RemoveComponent(uint32_t id) {
        m_meshComponents.RemoveElement(id);
    }

    MeshComponent JEMeshComponentManager::GetComponent(uint32_t index) const {
        return m_meshComponents[index];
    }

    void JEMeshComponentManager::SetComponent(uint32_t index, MeshComponent meshComp) {
        m_meshComponents[index] = meshComp;
    }

    const std::vector<MeshComponent>& JEMeshComponentManager::GetComponentList() const {
        return m_meshComponents.GetData();
    }
}
