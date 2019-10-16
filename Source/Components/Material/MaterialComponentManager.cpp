#include "MaterialComponentManager.h"

namespace JoeEngine {
    void JEMaterialComponentManager::Update(JEEngineInstance* engineInstance) {
        /*for (MaterialComponent comp : m_materialComponents) {
            // TODO
        }*/
    }

    void JEMaterialComponentManager::AddNewComponent(uint32_t id) {
        m_materialComponents.AddElement(id, MaterialComponent());
    }

    void JEMaterialComponentManager::RemoveComponent(uint32_t id) {
        m_materialComponents[id] = MaterialComponent();
        //m_materialComponents.RemoveElement(id);
    }

    MaterialComponent* JEMaterialComponentManager::GetComponent(uint32_t id) const {
        // This should be an ok const cast. The [-operator on std::vector only returns const refs.
        return const_cast<MaterialComponent*>(&m_materialComponents[id]);
    }

    void JEMaterialComponentManager::SetComponent(uint32_t id, MaterialComponent newComp) {
        m_materialComponents[id] = newComp;
    }

    const PackedArray<MaterialComponent>& JEMaterialComponentManager::GetComponentList() const {
        return m_materialComponents;
    }
}
