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
        ~JEMeshComponentManager() {}

        /*JEMeshComponentManager(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager(JEMeshComponentManager&& mgr) = delete;
        JEMeshComponentManager& operator=(const JEMeshComponentManager& mgr) = delete;
        JEMeshComponentManager& operator=(JEMeshComponentManager&& mgr) = delete;*/

        void Update(JEEngineInstance* engineInstance) override;
        void AddNewComponent(uint32_t id) override;
        void RemoveComponent(uint32_t id) override;

        MeshComponent* GetComponent(uint32_t id) const;
        void SetComponent(uint32_t id, MeshComponent meshComp);
        const std::vector<MeshComponent>& GetComponentList() const;
    };
}
