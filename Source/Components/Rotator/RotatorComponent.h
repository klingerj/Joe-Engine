#pragma once

#include "../../EngineInstance.h"

class RotatorComponent {
public:
    RotatorComponent() {}
    ~RotatorComponent() {}
    
    void Update(JoeEngine::JEEngineInstance* engineInstance, uint32_t id);
};
