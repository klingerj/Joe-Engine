#include "EntityManager.h"

namespace JoeEngine {

    Entity JEEntityManager::SpawnEntity() {
        Entity newEntity(++m_idCounter);
        m_entities.emplace_back(newEntity);
        m_entitiesDirty = true;
        return newEntity;
    }

    void JEEntityManager::DestroyEntity(Entity e) {
        // TODO: packed array remove ele @e.m_id
        m_entitiesDirty = true;
    }
}
