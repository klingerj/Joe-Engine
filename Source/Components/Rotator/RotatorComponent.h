#pragma once

#include "../../EngineInstance.h"

class RotatorComponent {
public:
    RotatorComponent() : m_entityId(-1) {}
    ~RotatorComponent() {}
    
    int m_entityId;

    void Update(JoeEngine::JEEngineInstance* engineInstance);
};
