#pragma once

#include "../ComponentManager.h"
#include "MaterialComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    class JEMaterialComponentManager : public JEComponentManager {
    private:
        PackedArray<MaterialComponent> m_materialComponents;

    public:
        JEMaterialComponentManager() {}
        virtual ~JEMaterialComponentManager() {}

        // Can't be copied/moved/assigned
        /*JEMaterialComponentManager(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager(JEMaterialComponentManager&& mgr) = delete;
        JEMaterialComponentManager& operator=(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager& operator=(JEMaterialComponentManager&& mgr) = delete;*/
        
        void Update(JEEngineInstance* engineInstance) override;
        void AddNewComponent(uint32_t id) override;
        void RemoveComponent(uint32_t id) override;

        MaterialComponent* GetComponent(uint32_t id) const;
        void SetComponent(uint32_t id, MaterialComponent newComp);
    };
}
