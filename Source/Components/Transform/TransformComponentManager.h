#pragma once

#include <vector>

#include "../ComponentManager.h"
#include "TransformComponent.h"

namespace JoeEngine {
    class JETransformComponentManager : public JEComponentManager {
    private:
        std::vector<TransformComponent> m_transformComponents;
    public:
        JETransformComponentManager() {
            m_transformComponents.reserve(128);
        }
        virtual ~JETransformComponentManager() {}

        /*JETransformComponentManager(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager(JETransformComponentManager&& mgr) = delete;
        JETransformComponentManager& operator=(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager& operator=(JETransformComponentManager&& mgr) = delete;*/

        void Update() override;
        void AddNewComponent() override;
        TransformComponent& GetComponent(uint32_t index);
        const std::vector<TransformComponent>& GetComponentList() const;
    };
}
