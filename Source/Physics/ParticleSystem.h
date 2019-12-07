#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "../Utils/RandomNumberGen.h"
#include "../Components/Mesh/MeshComponent.h"
#include "../Components/Material/MaterialComponent.h"
#include "../Rendering/VulkanRenderingTypes.h"
#include "../Utils/ScopedTimer.h"

namespace JoeEngine {
    typedef struct je_particle_system_settings_t {
        glm::vec3 position;
        float lifetime;
        uint32_t numParticles;
    } JEParticleSystemSettings;

    class JEParticleSystem {
    private:
        std::vector<glm::vec3> m_positionData;
        std::vector<glm::vec3> m_velocityData;
        std::vector<glm::vec3> m_accelData;
        std::vector<float> m_lifetimeData;
        const JEParticleSystemSettings m_settings;
        RNG::JERandomNumberGen<float> m_rng;
        MeshComponent m_meshComponent;
        std::vector<uint32_t> m_indices;
        MaterialComponent m_materialComponent;

        friend class JEPhysicsManager;
        friend class JEEngineInstance;

    public:
        JEParticleSystem() = delete;
        JEParticleSystem(const JEParticleSystemSettings& settings) :
            m_settings(settings), m_rng(0.0f, 1.0f), m_meshComponent(), m_materialComponent() {
            m_positionData.reserve(m_settings.numParticles);
            m_velocityData.reserve(m_settings.numParticles);
            m_accelData.reserve(m_settings.numParticles);
            m_lifetimeData.reserve(m_settings.numParticles);
            m_indices.reserve(m_settings.numParticles);
            for (uint32_t i = 0; i < m_settings.numParticles; ++i) {
                m_positionData.push_back(m_settings.position);
                m_velocityData.push_back(glm::normalize(glm::vec3(m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                    m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                    m_rng.GetNextRandomNum() * 2.0f - 1.0f)) * m_rng.GetNextRandomNum());
                m_accelData.push_back(glm::vec3(0.0f, -1.0f, 0.0f));
                m_lifetimeData.push_back(m_rng.GetNextRandomNum() * m_settings.lifetime);
                m_indices.push_back(i);
            }
        }
        ~JEParticleSystem() {}

        const std::vector<JEMeshPointVertex> GetVertices() {
            {
                //ScopedTimer<float> timer("Update Particle System Vertices");
                for (uint32_t i = 0; i < m_settings.numParticles; ++i) {
                    if (m_lifetimeData[i] < 0.0f) {
                        m_lifetimeData[i] = m_settings.lifetime;
                        m_positionData[i] = m_settings.position;
                        m_velocityData[i] = glm::normalize(glm::vec3(m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                            m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                            m_rng.GetNextRandomNum() * 2.0f - 1.0f)) * m_rng.GetNextRandomNum();
                    }
                }
            }
            return std::vector<JEMeshPointVertex>(m_positionData.data(), m_positionData.data() + m_settings.numParticles);
        }
        
        const std::vector<uint32_t>& GetIndices() const {
            return m_indices;
        }

        const MeshComponent& GetMeshComponent() const {
            return m_meshComponent;
        }

        const MaterialComponent& GetMaterialComponent() const {
            return m_materialComponent;
        }

        std::vector<glm::vec3>& GetPositionData() {
            return m_positionData;
        }

        std::vector<glm::vec3>& GetVelocityData() {
            return m_velocityData;
        }

        std::vector<glm::vec3>& GetAccelData() {
            return m_accelData;
        }

        std::vector<float>& GetLifetimeData() {
            return m_lifetimeData;
        }

    };
}
