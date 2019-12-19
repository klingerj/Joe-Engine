#include "MaterialComponentManager.h"

namespace JoeEngine {
    void JEMaterialComponentManager::Update(JEEngineInstance* engineInstance) {
        /*for (MaterialComponent& comp : m_materialComponents) {

        }*/
    }

    void JEMaterialComponentManager::AddNewComponent(uint32_t entityID) {
        m_materialComponents.AddElement(entityID, MaterialComponent());
    }

    void JEMaterialComponentManager::RemoveComponent(uint32_t id) {
        m_materialComponents[id] = MaterialComponent();
    }

    MaterialComponent* JEMaterialComponentManager::GetComponent(uint32_t entityID) const {
        // This should be an ok const cast. The [-operator on std::vector only returns const refs.
        return const_cast<MaterialComponent*>(&m_materialComponents[entityID]);
    }

    void JEMaterialComponentManager::SetComponent(uint32_t entityID, MaterialComponent newComp) {
        m_materialComponents[entityID] = newComp;
    }

    const PackedArray<MaterialComponent>& JEMaterialComponentManager::GetComponentList() const {
        return m_materialComponents;
    }
}
