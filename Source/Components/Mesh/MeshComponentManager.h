#pragma once

#include "../ComponentManager.h"
#include "MeshComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    //!  The Mesh Component Manager class
    /*!
      Contains all mesh components in a packed array of data.
      \sa JEEngineInstance
    */
    class JEMeshComponentManager : public JEComponentManager {
    private:
        //! Packed array of mesh components.
        /*! Manages all mesh component data. */
        PackedArray<MeshComponent> m_meshComponents;

    public:
        //! Default constructor.
        /*! No specific behavior. */
        JEMeshComponentManager() {}

        //! Destructor (default).
        virtual ~JEMeshComponentManager() = default;

        /*JEMeshComponentManager(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager(JEMeshComponentManager&& mgr) = delete;
        JEMeshComponentManager& operator=(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager& operator=(JEMeshComponentManager&& mgr) = delete;*/

        //! Update mesh components.
        /*!
          Updates all stored mesh components.
          Currently, updating mesh components does nothing.
          Overrides purely virtual function declared in JEComponentManager.
          \param engineInstance a reference to the current JEEngineInstance object if needed for certain API calls
        */
        void Update(JEEngineInstance* engineInstance) override;

        //! Add new mesh component.
        /*!
          Adds a new, default-constructed mesh component to the packed array of mesh components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to add the mesh component to
        */
        void AddNewComponent(uint32_t entityID) override;

        //! Remove mesh component.
        /*!
          Removes the mesh component from the packed array of mesh components
          at the specified entity index.
          Overrides purely virtual function declared in JEComponentManager.
          \param entityID the id of the entity to remove the mesh component from
        */
        void RemoveComponent(uint32_t entityID) override;

        //! Get mesh component.
        /*!
          Gets the mesh component attached to the entity ID.
          \param entityID the entity ID whose mesh component to return
          \return pointer to the mesh component attached to the entity ID
        */
        MeshComponent* GetComponent(uint32_t entityID) const;

        //! Set mesh component.
        /*!
          Sets the mesh component already attached to the entityID to the new component.
          \param entityID the entity ID whose mesh component to set
          \param meshComp the new material component
        */
        void SetComponent(uint32_t entityID, MeshComponent meshComp);

        //! Get list of mesh components.
        /*!
          Gets the member list of mesh components.
          \return the packed array of mesh components.
        */
        const PackedArray<MeshComponent>& GetComponentList() const;
    };
}
