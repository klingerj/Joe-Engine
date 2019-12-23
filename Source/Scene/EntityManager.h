#pragma once

#include "Entity.h"
#include "../Containers/PackedArray.h"

namespace JoeEngine {
    //! The Entity Manager class.
    /*!
      Class that manages the list of active entities in the scene. Currently, this class is not actually vital.
    */
    class JEEntityManager {
    private:
        //! Counts the number of entities to know the next valid entity id to use upon spawning.
        uint32_t m_idCounter;

        //! List of entities.
        PackedArray<Entity> m_entities;

    public:
        //! Constructor.
        /*! Initializes counter to zero. */
        JEEntityManager() : m_idCounter(0), m_entities() {}

        //! Destructor (default).
        ~JEEntityManager() = default;

        /*JEEntityManager(const JEEntityManager& mgr) = delete;
        JEEntityManager(const JEEntityManager&& mgr) = delete;
        JEEntityManager& operator=(const JEEntityManager& mgr) = delete;
        JEEntityManager& operator=(JEEntityManager&& mgr) = delete;*/

        //! Spawn entity.
        /*! Adds an entity to the list and increments the id counter. */
        Entity SpawnEntity();

        //! Destroy entity.
        /*! Removes a specific entity from the list. */
        void DestroyEntity(Entity e);

        //! Get current number of entities.
        uint32_t NumEntities() {
            return m_entities.Size();
        }
    };
}
