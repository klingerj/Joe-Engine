#pragma once

#include "../ComponentManager.h"
#include "RotatorComponent.h"
#include "../../Containers/PackedArray.h"

namespace JoeEngine {
    class JERotatorComponentManager : public JEComponentManager {
    private:
        PackedArray<RotatorComponent> m_rotatorComponents;

    public:
        JERotatorComponentManager() {}
        virtual ~JERotatorComponentManager() {}

        /*JETransformComponentManager(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager(JETransformComponentManager&& mgr) = delete;
        JETransformComponentManager& operator=(const JETransformComponentManager& mgr) = delete;
        JETransformComponentManager& operator=(JETransformComponentManager&& mgr) = delete;*/

        void Update() override;
        void AddNewComponent(uint32_t id) override;
        void RemoveComponent(uint32_t id) override;
    };
}
