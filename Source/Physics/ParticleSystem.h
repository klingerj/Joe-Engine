#pragma once

#include "glm/glm.hpp"

#include "../Containers/PackedArray.h"
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
        PackedArray<glm::vec3> m_positionData;
        PackedArray<glm::vec3> m_velocityData;
        PackedArray<glm::vec3> m_accelData;
        PackedArray<float> m_lifetimeData;
        const JEParticleSystemSettings m_settings;
        RNG::JERandomNumberGen<float> m_rng;
        MeshComponent m_meshComponent;
        std::vector<JEMeshVertex> m_vertices;
        std::vector<uint32_t> m_indices;
        MaterialComponent m_materialComponent;

        friend class JEPhysicsManager;
        friend class JEEngineInstance;

    public:
        JEParticleSystem() = delete;
        JEParticleSystem(const JEParticleSystemSettings& settings) :
            m_settings(settings), m_rng(0.0f, 1.0f), m_meshComponent(), m_materialComponent() {
            m_vertices.reserve(m_settings.numParticles);
            m_indices.reserve(m_settings.numParticles);
            for (uint32_t i = 0; i < m_settings.numParticles; ++i) {
                m_positionData.AddElement(i, m_settings.position);
                m_velocityData.AddElement(i, glm::normalize(glm::vec3(m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                    m_rng.GetNextRandomNum() * 2.0f - 1.0f,
                    m_rng.GetNextRandomNum() * 2.0f - 1.0f)) * m_rng.GetNextRandomNum());
                m_accelData.AddElement(i, glm::vec3(0.0f, -1.0f, 0.0f));
                m_lifetimeData.AddElement(i, m_rng.GetNextRandomNum() * m_settings.lifetime);
                m_indices.push_back(i);
                m_vertices.push_back({ { m_positionData[i] }, { glm::vec3(0.0f, 0.0f, 0.0f) }, { glm::vec3(1.0f, 1.0f, 1.0f) }, { glm::vec2(0.0f, 0.0f) } });
            }
        }
        ~JEParticleSystem() {}

        const std::vector<JEMeshVertex>& GetVertices() {
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
                    m_vertices[i].pos = m_positionData[i];
                }
            }
            return m_vertices;
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

        PackedArray<glm::vec3>& GetPositionData() {
            return m_positionData;
        }

        PackedArray<glm::vec3>& GetVelocityData() {
            return m_velocityData;
        }

        PackedArray<glm::vec3>& GetAccelData() {
            return m_accelData;
        }

        PackedArray<float>& GetLifetimeData() {
            return m_lifetimeData;
        }

    };
}
