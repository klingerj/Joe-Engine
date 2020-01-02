#pragma once

#include <stdint.h>

namespace JoeEngine {
    //! The Entity class.
    /*!
      Conceptually, an entity is just an index. This is the crux of the data-oriented design of the entity-component
      system of the Joe Engine. There is no other information stored within an Entity.
    */
    class Entity {
    private:
        //! Entity id.
        uint32_t m_id;

    public:
        //! Default constructor (deleted).
        Entity() = delete;

        //! Constructor.
        /*! Requires an id. */
        Entity(uint32_t id) : m_id(id) {}

        //! Destructor (default).
        ~Entity() = default;
        
        //! Get entity id.
        uint32_t GetId() const {
            return m_id;
        }
    };
}
