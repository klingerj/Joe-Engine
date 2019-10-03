#include "MaterialComponentManager.h"

namespace JoeEngine {
    void JEMaterialComponentManager::Update() {
        /*for (MaterialComponent comp : m_materialComponents) {
            // TODO
        }*/
    }

    void JEMaterialComponentManager::AddNewComponent() {
        m_materialComponents.emplace_back(MaterialComponent());
    }

    MaterialComponent JEMaterialComponentManager::GetComponent(uint32_t index) const {
        if (index > m_materialComponents.size() - 1) {
            //TODO: throw?
        }

        return m_materialComponents[index];
    }

    void JEMaterialComponentManager::SetComponent(uint32_t index, MaterialComponent newComp) {
        if (index > m_materialComponents.size() - 1) {
            //TODO: throw?
        }

        m_materialComponents[index] = newComp;
    }
}
