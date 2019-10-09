#pragma once

#include "../ComponentManager.h"
#include "TransformComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    //!  The Transform Component Manager class
    /*!
      Contains all transform components in a packed array of data.
      \sa JEEngineInstance
    */
    class JETransformComponentManager : public JEComponentManager {
    private:
        //! Packed array of transform components.
        /*! Manages all transform component data. */
        PackedArray<TransformComponent> m_transformComponents;

    public:
        //! Default constructor.
        /*! No specific behavior. */
        JETransformComponentManager() = default;

        //! Destructor (default).
        virtual ~JETransformComponentManager() = default;

        /*JETransformComponentManager(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager(JETransformComponentManager&& mgr) = delete;
        JETransformComponentManager& operator=(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager& operator=(JETransformComponentManager&& mgr) = delete;*/

        //! Update transform components.
        /*!
          Updates all stored transform components.
          Currently, updating transform components does nothing.
          Overrides purely virtual function declared in JEComponentManager.
          \param engineInstance a reference to the current JEEngineInstance object if needed for certain API calls
        */
        void Update(JEEngineInstance* engineInstance) override;

        //! Add new transform component.
        /*!
          Adds a new, default-constructed transform component to the packed array of transform components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to add the transform component to
        */
        void AddNewComponent(uint32_t id) override;

        //! Remove transform component.
        /*!
          Removes the transform component from the packed array of transform components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to remove the transform component from
        */
        void RemoveComponent(uint32_t id) override;

        //! Get transform component.
        /*!
          Gets the transform component attached to the entity ID.
          \param entityID the entity ID whose transform component to return
          \return pointer to the transform component attached to the entity ID
        */
        TransformComponent* GetComponent(uint32_t id) const;

        //! Get list of transform components.
        /*!
          Gets the member list of transform components.
          \return the packed array of transform components.
        */
        const PackedArray<TransformComponent>& GetComponentList() const;
    };
}
