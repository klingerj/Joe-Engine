#pragma once

#include "Entity.h"
#include "../Containers/PackedArray.h"

namespace JoeEngine {
    class JEEntityManager {
    private:
        uint32_t m_idCounter;

        // List of entities
        PackedArray<Entity> m_entities;

        // If the list of entities changes, this bool will be true until marked clean
        bool m_entitiesDirty;

    public:
        JEEntityManager() : m_idCounter(0), m_entities(), m_entitiesDirty(false) {}
        ~JEEntityManager() {}

        JEEntityManager(const JEEntityManager& mgr) = delete;
        JEEntityManager(const JEEntityManager&& mgr) = delete;
        JEEntityManager& operator=(const JEEntityManager& mgr) = delete;
        JEEntityManager& operator=(JEEntityManager&& mgr) = delete;

        Entity SpawnEntity();
        void DestroyEntity(Entity e);

        uint32_t NumEntities() {
            return m_entities.Size();
        }

        const bool& EntitiesDirty() const {
            return m_entitiesDirty;
        }
        void EntitiesClean() {
            m_entitiesDirty = false;
        }
    };
}
