#pragma once

#include <stdint.h>

namespace JoeEngine {
    class Entity {
    public:
        Entity(uint32_t id) : m_id(id) {}
        Entity() = delete;
        ~Entity() {}
        
        const uint32_t m_id;
    };
}
