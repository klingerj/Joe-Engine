#pragma once

#include "../ComponentManager.h"
#include "RotatorComponent.h"
#include "../../Containers/PackedArray.h"

//! The Rotator Component Manager class
/*!
  User-defined class that contains all rotator components in a packed array of data.
  \sa JEEngineInstance
*/
class RotatorComponentManager : public JoeEngine::JEComponentManager {
private:
    //! Packed array of rotator components.
    /*! Manages all rotator component data. */
    JoeEngine::PackedArray<RotatorComponent> m_rotatorComponents;

public:
    //! Default constructor.
    /*! No specific behavior. */
    RotatorComponentManager() = default;

    //! Destructor (default).
    virtual ~RotatorComponentManager() = default;

    /*RotatorComponentManager(const RotatorComponentManager& mgr) = delete;
    RotatorComponentManager(RotatorComponentManager&& mgr) = delete;
    RotatorComponentManager& operator=(const RotatorComponentManager& mgr) = delete;
    RotatorComponentManager& operator=(RotatorComponentManager&& mgr) = delete;*/

    //! Update rotator components.
    /*!
      Updates all stored rotator components.
      Overrides purely virtual function declared in JEComponentManager.
      \param engineInstance a reference to the current JEEngineInstance object if needed for certain API calls
    */
    void Update(JoeEngine::JEEngineInstance* engineInstance) override;

    //! Add new rotator component.
    /*!
      Adds a new, default-constructed rotator component to the packed array of rotator components
      at the specified entity index.
      Overrides purely virtual function declared in JEComponentManager.
      \param entityID the id of the entity to add the rotator component to
    */
    void AddNewComponent(uint32_t id) override;

    //! Remove rotator component.
    /*!
      Removes the rotator component from the list of rotator components
      at the specified entity index.
      Overrides purely virtual function declared in JEComponentManager.
      \param entityID the id of the entity to remove the rotator component from
    */
    void RemoveComponent(uint32_t id) override;

    //! Get rotator component.
    /*!
      Gets the rotator component attached to the entity ID.
      \param entityID the entity ID whose rotator component to return
      \return pointer to the rotator component attached to the entity ID
    */
    RotatorComponent* GetComponent(uint32_t index) const;
};
