#include "RotatorComponentManager.h"

void RotatorComponentManager::Update(JoeEngine::JEEngineInstance* engineInstance) {
    uint32_t i = 0;
    for (RotatorComponent& r : m_rotatorComponents) {
        r.Update(engineInstance, i);
        ++i;
    }
}

void RotatorComponentManager::AddNewComponent(uint32_t id) {
    m_rotatorComponents.AddElement(id, RotatorComponent());
}

void RotatorComponentManager::RemoveComponent(uint32_t id) {
    m_rotatorComponents.RemoveElement(id);
}

RotatorComponent* RotatorComponentManager::GetComponent(uint32_t id) const {
    // This should be an ok const cast. The [-operator on std::vector only returns const refs.
    return const_cast<RotatorComponent*>(&m_rotatorComponents[id]);
}
