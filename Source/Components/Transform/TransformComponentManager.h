#pragma once

#include "../ComponentManager.h"
#include "TransformComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    class JETransformComponentManager : public JEComponentManager {
    private:
        PackedArray<TransformComponent> m_transformComponents;

    public:
        JETransformComponentManager() {}
        virtual ~JETransformComponentManager() {}

        /*JETransformComponentManager(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager(JETransformComponentManager&& mgr) = delete;
        JETransformComponentManager& operator=(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager& operator=(JETransformComponentManager&& mgr) = delete;*/

        void Update() override;
        void AddNewComponent(uint32_t id) override;
        void RemoveComponent(uint32_t id) override;

        TransformComponent* GetComponent(uint32_t index);
        const std::vector<TransformComponent>& GetComponentList() const;
    };
}
