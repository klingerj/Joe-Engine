#include "EntityManager.h"

namespace JoeEngine {
    Entity JEEntityManager::SpawnEntity() {
        Entity newEntity(m_idCounter++);
        m_entities.AddElement(newEntity.GetId(), newEntity);
        return newEntity;
    }

    void JEEntityManager::DestroyEntity(Entity e) {
        m_entities.RemoveElement(e.GetId());
    }
}
