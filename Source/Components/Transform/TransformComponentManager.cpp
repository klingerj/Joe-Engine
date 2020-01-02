#include "TransformComponentManager.h"

namespace JoeEngine {
    void JETransformComponentManager::Update(JEEngineInstance* engineInstance) {
        /*for (TransformComponent& t : m_transformComponents) {

        }*/
    }

    void JETransformComponentManager::AddNewComponent(uint32_t id) {
        m_transformComponents.AddElement(id, TransformComponent());
    }

    void JETransformComponentManager::RemoveComponent(uint32_t id) {
        m_transformComponents[id] = TransformComponent();
        //m_transformComponents.RemoveElement(id);
    }

    TransformComponent* JETransformComponentManager::GetComponent(uint32_t id) const {
        // This should be an ok const cast. The [-operator on std::vector only returns const refs.
        return const_cast<TransformComponent*>(&m_transformComponents[id]);
    }

    const PackedArray<TransformComponent>& JETransformComponentManager::GetComponentList() const {
        return m_transformComponents;
    }
}
