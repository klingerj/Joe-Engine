#pragma once

#include <stdint.h>

namespace JoeEngine {
    class Entity {
    private:
        uint32_t m_id;

    public:
        Entity(uint32_t id) : m_id(id) {}
        Entity() = delete;
        ~Entity() {}
        
        uint32_t GetId() const {
            return m_id;
        }
    };
}
