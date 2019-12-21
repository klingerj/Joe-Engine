#pragma once

#include <vector>

#include "glm/glm.hpp"

#include "../Utils/RandomNumberGen.h"
#include "../Components/Mesh/MeshComponent.h"
#include "../Components/Material/MaterialComponent.h"
#include "../Rendering/VulkanRenderingTypes.h"
#include "../Utils/ScopedTimer.h"

namespace JoeEngine {
    //! Particle settings struct.
    /*! Data that specifies all possible settings necessary to create a particle system. */
    typedef struct je_particle_system_settings_t {
        glm::vec3 position;
        float lifetime;
        uint32_t numParticles;
    } JEParticleSystemSettings;

    //! The Particle System class.
    /*!
      Class that manages the data for a particle system. Position/velocity/acceleration data are all stored in their own 
      linear, contiguous memory containers. Also manages rendering resources like Mesh/Material Components (should change
      in the future).
    */
    class JEParticleSystem {
    private:
        
        //! Position data.
        std::vector<glm::vec3> m_positionData;
        
        //! Velocity data.
        std::vector<glm::vec3> m_velocityData;

        //! Acceleration data.
        std::vector<glm::vec3> m_accelData;

        //! Lifetime data.
        std::vector<float> m_lifetimeData;

        //! Settings for this particle system.
        const JEParticleSystemSettings m_settings;

        //! Random number generator.
        RNG::JERandomNumberGen<float> m_rng;

        //! Mesh component for rendering.
        MeshComponent m_meshComponent;

        //! Indices list for rendering.
        std::vector<uint32_t> m_indices;

        //! Material component for rendering.
        MaterialComponent m_materialComponent;

        //! Physics manager can access private members.
        friend class JEPhysicsManager;

        //! The engine instance can access private members.
        friend class JEEngineInstance;

    public:
        //! Default constructor (deleted).
        JEParticleSystem() = delete;

        //! Constructor.
        /*! Initializes important data and populates the various particle data lists. */
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

        //! Destructor (default).
        ~JEParticleSystem() = default;

        //! Get particle vertices.
        /*!
          Return an up-to-date list of particle positions for rendering. Also resets particle data if lifetime is over.
          Note: particles not deleted when their lifetime is over, they just reset to the origin of the system with a 
          new lifetime.
        */
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
        
        //! Get indices list.
        const std::vector<uint32_t>& GetIndices() const {
            return m_indices;
        }

        //! Get mesh component.
        const MeshComponent& GetMeshComponent() const {
            return m_meshComponent;
        }

        //! Get material component.
        const MaterialComponent& GetMaterialComponent() const {
            return m_materialComponent;
        }

        //! Get all position data.
        std::vector<glm::vec3>& GetPositionData() {
            return m_positionData;
        }

        //! Get all velocity data.
        std::vector<glm::vec3>& GetVelocityData() {
            return m_velocityData;
        }

        //! Get all acceleration data.
        std::vector<glm::vec3>& GetAccelData() {
            return m_accelData;
        }

        //! Get all lifetime data.
        std::vector<float>& GetLifetimeData() {
            return m_lifetimeData;
        }

        //! Get number of particles in system (never changes).
        uint32_t GetNumParticles() const {
            return m_settings.numParticles;
        }
    };
}
