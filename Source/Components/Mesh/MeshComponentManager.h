#pragma once

#include "../ComponentManager.h"
#include "MeshComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    class JEMeshComponentManager : public JEComponentManager {
    private:
        PackedArray<MeshComponent> m_meshComponents;
    public:
        JEMeshComponentManager() {}
        virtual ~JEMeshComponentManager() {}

        /*JEMeshComponentManager(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager(JEMeshComponentManager&& mgr) = delete;
        JEMeshComponentManager& operator=(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager& operator=(JEMeshComponentManager&& mgr) = delete;*/

        void Update() override;
        void AddNewComponent(uint32_t id) override;
        void RemoveComponent(uint32_t id) override;

        MeshComponent GetComponent(uint32_t index) const;
        void SetComponent(uint32_t index, MeshComponent meshComp);
        const std::vector<MeshComponent>& GetComponentList() const;
    };
}
