#pragma once

#include "../ComponentManager.h"
#include "MaterialComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    //!  The Material Component Manager class
    /*!
      Contains all material components in a packed array of data.
      The material system is in progress and this class may change in the near future.
      \sa JEEngineInstance
    */
    class JEMaterialComponentManager : public JEComponentManager {
    private:
        //! Packed array of material components.
        /*! Manages all material component data. */
        PackedArray<MaterialComponent> m_materialComponents;

    public:
        //! Default constructor.
        /*! No specific behavior. */
        JEMaterialComponentManager() = default;

        //! Destructor (default).
        virtual ~JEMaterialComponentManager() = default;

        // Can't be copied/moved/assigned
        /*JEMaterialComponentManager(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager(JEMaterialComponentManager&& mgr) = delete;
        JEMaterialComponentManager& operator=(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager& operator=(JEMaterialComponentManager&& mgr) = delete;*/
        
        //! Update material components.
        /*!
          Updates all stored material components.
          Currently, updating material components does nothing.
          Overrides purely virtual function declared in JEComponentManager.
          \param engineInstance a reference to the current JEEngineInstance object if needed for certain API calls
        */
        void Update(JEEngineInstance* engineInstance) override;
        
        //! Add new material component.
        /*!
          Adds a new, default-constructed material component to the packed array of material components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to add the material component to
        */
        void AddNewComponent(uint32_t entityID) override;

        //! Remove material component.
        /*!
          Removes the material component from the packed array of material components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to remove the material component from
        */
        void RemoveComponent(uint32_t entityID) override;

        //! Get material component.
        /*!
          Gets the material component attached to the entity ID.
          \param entityID the entity ID whose material component to return
          \return pointer to the material component attached to the entity ID
        */
        MaterialComponent* GetComponent(uint32_t entityID) const;

        //! Set material component.
        /*!
          Sets the material component already attached to the entityID to the new component.
          \param entityID the entity ID whose material component to set
          \param newComp the new material component
        */
        void SetComponent(uint32_t entityID, MaterialComponent newComp);


        const PackedArray<MaterialComponent>& GetComponentList() const;
    };
}
