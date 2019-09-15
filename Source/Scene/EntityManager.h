#pragma once

#include <vector>

#include "Entity.h"

namespace JoeEngine {
    class JEEntityManager {
    private:
        uint32_t m_idCounter;

        // TODO: replace with packed array
        // list of entities, tightly packed
        std::vector<Entity> m_entities;

        // if the list of entities changes, this bool will be true until marked clean
        bool m_entitiesDirty;

    public:
        JEEntityManager() : m_idCounter(0), m_entities(), m_entitiesDirty(false) {
            m_entities.reserve(128);
        }
        ~JEEntityManager() {}

        JEEntityManager(const JEEntityManager& mgr) = delete;
        JEEntityManager(const JEEntityManager&& mgr) = delete;
        JEEntityManager& operator=(const JEEntityManager& mgr) = delete;
        JEEntityManager& operator=(JEEntityManager&& mgr) = delete;

        Entity SpawnEntity();
        void DestroyEntity(Entity e);

        uint32_t NumEntities() {
            return m_entities.size(); // TODO: this will not be right once entity destruction is implemented
        }

        const bool& EntitiesDirty() const {
            return m_entitiesDirty;
        }
        void EntitiesClean() {
            m_entitiesDirty = false;
        }
    };
}
