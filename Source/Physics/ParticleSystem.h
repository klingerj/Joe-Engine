#pragma once

#include "glm/glm.hpp"

#include "../Containers/PackedArray.h"

namespace JoeEngine {
    class JEParticleSystem {
    private:
        // stuff
        PackedArray<glm::vec3> m_positionData;

    public:
        JEParticleSystem() {}
        ~JEParticleSystem() {}

    };
}
