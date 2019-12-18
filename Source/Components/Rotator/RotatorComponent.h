#pragma once

#include "../../EngineInstance.h"

//! The Rotator Component class
/*!
  A custom-made component type. Note that it exists outside of the usual JoeEngine namespace.\
  When attached to an entity, the entity should rotate about a specified axis.
  \sa RotatorComponentManager
*/
class RotatorComponent {
public:

    //! Constructor.
    /*! Constructs a rotator component with default values. */
    RotatorComponent() : m_entityId(-1), m_axis(0, 0, 1) {}
    
    //! Destructor (default).
    ~RotatorComponent() = default;
    
    //! Entity Id.
    /*! Indicates which entity this component is attached to. */
    int m_entityId;

    //! Axis of rotation.
    /*! Indicates which axis to rotate about. */
    glm::vec3 m_axis;

    //! Update this rotator component.
    /*!
      A user-defined function that should invoke some custom functionality. In this case, it should transform the
      attached entity (stored in m_entityId) by rotating it about m_axis.
      \param engineInstance the engine instance (needed for general API calls).
    */
    void Update(JoeEngine::JEEngineInstance* engineInstance);
};
