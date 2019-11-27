#pragma once

#include "glm/glm.hpp"

#include "../Containers/PackedArray.h"
#include "../Utils/RandomNumberGen.h"

namespace JoeEngine {
    typedef struct je_particle_system_settings_t {
        glm::vec3 position;
        float lifetime;
        uint32_t numParticles;
    } JEParticleSystemSettings;

    class JEParticleSystem {
    private:
        PackedArray<glm::vec3> m_positionData;
        PackedArray<glm::vec3> m_velocityData;
        PackedArray<glm::vec3> m_accelData;
        PackedArray<float> m_lifetimeData;
        uint32_t m_numParticles;
        RNG::JERandomNumberGen<float> rng;

        friend class JEPhysicsManager;

    public:
        JEParticleSystem() = delete;
        JEParticleSystem(const JEParticleSystemSettings& settings) : m_numParticles(settings.numParticles), rng(-5.0f, 5.0f) {
            for (uint32_t i = 0; i < m_numParticles; ++i) {
                m_positionData.AddElement(i, settings.position);
                m_velocityData.AddElement(i, glm::vec3(rng.GetNextRandomNum(),
                                                       rng.GetNextRandomNum(),
                                                       rng.GetNextRandomNum()));
                m_accelData.AddElement(i, glm::vec3(0.0f, -0.5f, 0.0f));
                m_lifetimeData.AddElement(i, settings.lifetime);
            }
        }
        ~JEParticleSystem() {}

    };
}
