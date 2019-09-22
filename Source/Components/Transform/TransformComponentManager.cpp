#include "TransformComponentManager.h"

namespace JoeEngine {
    void JETransformComponentManager::Update() {
        for (TransformComponent t : m_transformComponents) {
            // TODO: something
        }
    }

    void JETransformComponentManager::AddNewComponent() {
        m_transformComponents.emplace_back(TransformComponent());
    }

    TransformComponent& JETransformComponentManager::GetComponent(uint32_t index) {
        if (index > m_transformComponents.size() - 1) {
            //TODO: throw exception or something
        }
        return m_transformComponents[index];
    }

    const std::vector<TransformComponent>& JETransformComponentManager::GetComponentList() const {
        return m_transformComponents;
    }
}
