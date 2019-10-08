#pragma once

#include "../ComponentManager.h"
#include "RotatorComponent.h"
#include "../../Containers/PackedArray.h"

class RotatorComponentManager : public JoeEngine::JEComponentManager {
private:
    JoeEngine::PackedArray<RotatorComponent> m_rotatorComponents;

public:
    RotatorComponentManager() {}
    virtual ~RotatorComponentManager() {}

    /*JETransformComponentManager(const JETransformComponentManager& mgr) = delete;
    JETransformComponentManager(JETransformComponentManager&& mgr) = delete;
    JETransformComponentManager& operator=(const JETransformComponentManager& mgr) = delete;
    JETransformComponentManager& operator=(JETransformComponentManager&& mgr) = delete;*/

    void Update(JoeEngine::JEEngineInstance* engineInstance) override;
    void AddNewComponent(uint32_t id) override;
    void RemoveComponent(uint32_t id) override;
    RotatorComponent* GetComponent(uint32_t index) const;
};
