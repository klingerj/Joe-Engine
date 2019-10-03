#pragma once

#include <vector>

#include "../ComponentManager.h"
#include "MaterialComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    class JEMaterialComponentManager : public JEComponentManager {
    private:
        std::vector<MaterialComponent> m_materialComponents;
        PackedArray<MaterialComponent> m_materialComponents_packed;

    public:
        JEMaterialComponentManager() {}
        virtual ~JEMaterialComponentManager() {}

        // Can't be copied/moved/assigned
        /*JEMaterialComponentManager(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager(JEMaterialComponentManager&& mgr) = delete;
        JEMaterialComponentManager& operator=(const JEMaterialComponentManager& mgr) = delete;
        JEMaterialComponentManager& operator=(JEMaterialComponentManager&& mgr) = delete;*/
        
        void Update() override;
        void AddNewComponent() override;

        MaterialComponent GetComponent(uint32_t index) const;
        void SetComponent(uint32_t index, MaterialComponent newComp);
    };
}
