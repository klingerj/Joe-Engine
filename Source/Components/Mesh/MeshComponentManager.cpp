#include "MeshComponentManager.h"

namespace JoeEngine {
    void JEMeshComponentManager::Update(JEEngineInstance* engineInstance) {
        /*for (MeshComponent& m : m_meshComponents) {
            // TODO
        }*/
    }

    void JEMeshComponentManager::AddNewComponent(uint32_t id) {
        m_meshComponents.AddElement(id, MeshComponent());
    }

    void JEMeshComponentManager::RemoveComponent(uint32_t id) {
        m_meshComponents[id] = MeshComponent(-1, MESH_TRIANGLES);
        //m_meshComponents.RemoveElement(id);
    }

    MeshComponent* JEMeshComponentManager::GetComponent(uint32_t id) const {
        // This should be an ok const cast. The [-operator on std::vector only returns const refs.
        return const_cast<MeshComponent*>(&m_meshComponents[id]);
    }

    void JEMeshComponentManager::SetComponent(uint32_t id, MeshComponent meshComp) {
        m_meshComponents[id] = meshComp;
    }

    const PackedArray<MeshComponent>& JEMeshComponentManager::GetComponentList() const {
        return m_meshComponents;
    }
}
