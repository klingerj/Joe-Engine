#pragma once

#include "Entity.h"
#include "../Containers/PackedArray.h"

namespace JoeEngine {
    class JEEntityManager {
    private:
        uint32_t m_idCounter;

        // List of entities
        PackedArray<Entity> m_entities;

    public:
        JEEntityManager() : m_idCounter(0), m_entities() {}
        ~JEEntityManager() {}

        /*JEEntityManager(const JEEntityManager& mgr) = delete;
        JEEntityManager(const JEEntityManager&& mgr) = delete;
        JEEntityManager& operator=(const JEEntityManager& mgr) = delete;
        JEEntityManager& operator=(JEEntityManager&& mgr) = delete;*/

        Entity SpawnEntity();
        void DestroyEntity(Entity e);

        uint32_t NumEntities() {
            return m_entities.Size();
        }
    };
}
