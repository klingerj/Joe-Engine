#pragma once

#include <stdint.h>

namespace JoeEngine {
    class JEEngineInstance;

    class JEComponentManager {
    public:
        JEComponentManager() {}
        ~JEComponentManager() {}

        virtual void Update(JEEngineInstance* engineInstance) = 0;
        virtual void AddNewComponent(uint32_t entityID) = 0;
        virtual void RemoveComponent(uint32_t entityID) = 0;
    };
}
