#pragma once

#include <stdint.h>

namespace JoeEngine {
    enum JE_SHADER_TYPE : uint8_t {
        FORWARD = 0,
        DEFERRED = 1,
        CUSTOM = 2
    };

    class MaterialComponent {
    public:
        int shaderID;
        JE_SHADER_TYPE shaderType;

        MaterialComponent() : MaterialComponent(-1, DEFERRED) {}
        MaterialComponent(uint16_t id, JE_SHADER_TYPE t) : shaderID(id), shaderType(t) {}
        ~MaterialComponent() {}
    };
}
