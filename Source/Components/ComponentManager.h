#pragma once

#include <stdint.h>

namespace JoeEngine {
    class JEEngineInstance;

    //!  The Component Manager class
    /*!
      Abstract base class for all component manager derived classes.
      \sa JEEngineInstance
    */
    class JEComponentManager {
    public:
        //! Default constructor.
        /*! No specific behavior. */
        JEComponentManager() = default;

        //! Destructor (default).
        virtual ~JEComponentManager() = default;

        //! Update components.
        /*!
          Updates all stored components.
          Purely virtual function to be overridden by derived component manager classes.
          \param engineInstance a reference to the current JEEngineInstance object if needed for certain API calls
        */
        virtual void Update(JEEngineInstance* engineInstance) = 0;
        
        //! Add new component.
        /*!
          Adds a new component to the stored list of components at the specified entity index.
          Purely virtual function to be overridden by derived component manager classes.
          \param entityID the id of the entity to add the transform component to
        */
        virtual void AddNewComponent(uint32_t entityID) = 0;
        
        //! Remove component.
        /*!
          Removes the component from the list of transform components at the specified entity index.
          Purely virtual function to be overridden by derived component manager classes.
          \param entityID the id of the entity to remove the transform component from
        */
        virtual void RemoveComponent(uint32_t entityID) = 0;
    };
}
