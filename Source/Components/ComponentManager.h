#pragma once

#include <stdint.h>

namespace JoeEngine {
    class JEComponentManager {
    public:
        JEComponentManager() {}
        virtual ~JEComponentManager() {}

        virtual void Update() = 0;
        virtual void AddNewComponent(uint32_t id) = 0;
        virtual void RemoveComponent(uint32_t id) = 0;
    };
}
