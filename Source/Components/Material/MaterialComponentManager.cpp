#include "MaterialComponentManager.h"

namespace JoeEngine {
    void JEMaterialComponentManager::Update() {
        /*for (MaterialComponent comp : m_materialComponents) {
            // TODO
        }*/
    }

    void JEMaterialComponentManager::AddNewComponent(uint32_t id) {
        m_materialComponents.AddElement(id, MaterialComponent());
    }

    void JEMaterialComponentManager::RemoveComponent(uint32_t id) {
        m_materialComponents.RemoveElement(id);
    }

    MaterialComponent JEMaterialComponentManager::GetComponent(uint32_t index) const {
        return m_materialComponents[index];
    }

    void JEMaterialComponentManager::SetComponent(uint32_t index, MaterialComponent newComp) {
        m_materialComponents[index] = newComp;
    }
}
