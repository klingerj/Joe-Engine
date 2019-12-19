#include "RotatorComponentManager.h"

void RotatorComponentManager::Update(JoeEngine::JEEngineInstance* engineInstance) {
    for (RotatorComponent& r : m_rotatorComponents) {
        r.Update(engineInstance);
    }
}

void RotatorComponentManager::AddNewComponent(uint32_t entityID) {
    m_rotatorComponents.AddElement(entityID, RotatorComponent());
}

void RotatorComponentManager::RemoveComponent(uint32_t entityID) {
    m_rotatorComponents.RemoveElement(entityID);
}

RotatorComponent* RotatorComponentManager::GetComponent(uint32_t id) const {
    // This should be an ok const cast. The [-operator on std::vector only returns const refs.
    return const_cast<RotatorComponent*>(&m_rotatorComponents[id]);
}
