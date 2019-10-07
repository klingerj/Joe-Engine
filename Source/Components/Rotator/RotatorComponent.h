#pragma once

#include "../../EngineInstance.h"

class RotatorComponent {
public:
    RotatorComponent() : m_entityId(-1), m_axis(0, 0, 1) {}
    ~RotatorComponent() {}
    
    int m_entityId;
    glm::vec3 m_axis;

    void Update(JoeEngine::JEEngineInstance* engineInstance);
};
