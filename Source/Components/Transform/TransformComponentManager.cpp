#include "TransformComponentManager.h"

namespace JoeEngine {
    void JETransformComponentManager::Update() {
        /*for (TransformComponent& t : m_transformComponents) {
            // TODO
        }*/
    }

    void JETransformComponentManager::AddNewComponent(uint32_t id) {
        m_transformComponents.AddElement(id, TransformComponent());
    }

    void JETransformComponentManager::RemoveComponent(uint32_t id) {
        m_transformComponents.RemoveElement(id);
    }

    TransformComponent* JETransformComponentManager::GetComponent(uint32_t index) {
        return &m_transformComponents[index];
    }

    const std::vector<TransformComponent>& JETransformComponentManager::GetComponentList() const {
        return m_transformComponents.GetData();
    }
}
