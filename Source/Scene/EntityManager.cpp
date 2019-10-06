#include "EntityManager.h"

namespace JoeEngine {
    Entity JEEntityManager::SpawnEntity() {
        Entity newEntity(m_idCounter++);
        m_entities.AddElement(newEntity.GetId(), newEntity);
        m_entitiesDirty = true;
        return newEntity;
    }

    void JEEntityManager::DestroyEntity(Entity e) {
        m_entities.RemoveElement(e.GetId());
        m_entitiesDirty = true;
    }
}
