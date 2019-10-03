#include "RotatorComponentManager.h"

namespace JoeEngine {
    void JERotatorComponentManager::Update() {
        for (RotatorComponent& r : m_rotatorComponents) {
            r.Update();
        }
    }

    void JERotatorComponentManager::AddNewComponent(uint32_t id) {
        m_rotatorComponents.AddElement(id, RotatorComponent());
    }

    void JERotatorComponentManager::RemoveComponent(uint32_t id) {
        m_rotatorComponents.RemoveElement(id);
    }
}
