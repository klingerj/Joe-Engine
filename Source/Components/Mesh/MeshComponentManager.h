#pragma once

#include <vector>

#include "../ComponentManager.h"
#include "MeshComponent.h"

namespace JoeEngine {
    class JEMeshComponentManager : public JEComponentManager {
    private:
        std::vector<MeshComponent> m_meshComponents;
    public:
        JEMeshComponentManager() {
            m_meshComponents.reserve(128);
        }
        virtual ~JEMeshComponentManager() {}

        /*JEMeshComponentManager(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager(JEMeshComponentManager&& mgr) = delete;
        JEMeshComponentManager& operator=(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager& operator=(JEMeshComponentManager&& mgr) = delete;*/

        void Update() override;
        void AddNewComponent() override;

        MeshComponent GetComponent(uint32_t index) const;
        void SetComponent(uint32_t index, MeshComponent meshComp);
        const std::vector<MeshComponent>& GetComponentList() const;
    };
}
