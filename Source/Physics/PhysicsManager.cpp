#ifndef JOE_ENGINE_SIMD_NONE
#include <immintrin.h>
#endif

#include "glm/gtc/epsilon.hpp"
#include "glm/gtx/norm.hpp"

#include "JoeEngineConfig.h"
#include "PhysicsManager.h"
#include "../Utils/ThreadPool.h"

namespace JoeEngine {
    void JEPhysicsManager::Initialize() {
        m_startTime = std::chrono::high_resolution_clock::now();
    }

    typedef struct particle_update_data_t {
        JEParticleSystem* particleSystem;
        float dt;
        uint32_t startIdx;
        uint32_t endIdx;
        bool complete;
    } ParticleUpdateData;

    // Multithreading functions for particle updates
    void UpdateParticleSystems_MT(void* data) {
        ParticleUpdateData* particleData = (ParticleUpdateData*)data;

        std::vector<glm::vec3>& positions = particleData->particleSystem->GetPositionData();
        std::vector<glm::vec3>& velocities = particleData->particleSystem->GetVelocityData();
        std::vector<glm::vec3>& accels = particleData->particleSystem->GetAccelData();
        std::vector<float>& lifetimes = particleData->particleSystem->GetLifetimeData();
        const uint32_t numParticlesToUpdate = particleData->endIdx - particleData->startIdx;

        #ifdef JOE_ENGINE_SIMD_AVX2
        // Use AVX2 SIMD commands to integrate multiple particles at a time
        const uint8_t groupSize = 2;
        uint32_t numGroups = numParticlesToUpdate / groupSize;
        for (uint32_t i = 0; i < numGroups; ++i) {
            uint32_t offset = i * groupSize + particleData->startIdx;
            // Create vector for dt float
            __m256 dtData = _mm256_set1_ps(particleData->dt);

            // Copy velocity and acceleration data into registers
            __m256 velData = _mm256_setr_ps(velocities[offset].x, velocities[offset].y, velocities[offset].z,
                velocities[offset + 1].x, velocities[offset + 1].y, velocities[offset + 1].z, 0.0f, 0.0f);
            __m256 accelData = _mm256_setr_ps(accels[offset].x, accels[offset].y, accels[offset].z,
                accels[offset + 1].x, accels[offset + 1].y, accels[offset + 1].z, 0.0f, 0.0f);

            // Scale acceleration by dt
            __m256 accelDt = _mm256_mul_ps(dtData, accelData);
            // Add result to velocity
            __m256 velDataUpdated = _mm256_add_ps(accelDt, velData);

            // Copy position data
            __m256 posData = _mm256_setr_ps(positions[offset].x, positions[offset].y, positions[offset].z,
                positions[offset + 1].x, positions[offset + 1].y, positions[offset + 1].z, 0.0f, 0.0f);
            // Scale velocity by dt
            __m256 velDt = _mm256_mul_ps(dtData, velDataUpdated);
            // Add result to position
            __m256 posDataUpdated = _mm256_add_ps(velDt, posData);

            // Copy results back to particle system
            float *velDataPtr = (float*)&velDataUpdated;
            velocities[offset].x = velDataPtr[0];
            velocities[offset].y = velDataPtr[1];
            velocities[offset].z = velDataPtr[2];
            velocities[offset + 1].x = velDataPtr[3];
            velocities[offset + 1].y = velDataPtr[4];
            velocities[offset + 1].z = velDataPtr[5];

            float *posDataPtr = (float*)&posDataUpdated;
            positions[offset].x = posDataPtr[0];
            positions[offset].y = posDataPtr[1];
            positions[offset].z = posDataPtr[2];
            positions[offset + 1].x = posDataPtr[3];
            positions[offset + 1].y = posDataPtr[4];
            positions[offset + 1].z = posDataPtr[5];
        }

        // Integrate leftover particles
        if (numParticlesToUpdate % groupSize != 0) {
            uint32_t j = numGroups * groupSize + particleData->startIdx;
            for (; j < particleData->endIdx; ++j) {
                velocities[j] += accels[j] * particleData->dt;
                positions[j] += velocities[j] * particleData->dt;
            }
        }

        // Update particle lifetimes
        // TODO: make this use SIMD
        const float lifetimeDecrement = particleData->dt * 1000;
        for (uint32_t i = particleData->startIdx; i < particleData->endIdx; ++i) {
            lifetimes[i] -= lifetimeDecrement;
        }
        #endif
        
        #ifdef JOE_ENGINE_SIMD_NONE
        // Update velocities
        for (uint32_t i = particleData->startIdx; i < particleData->endIdx; ++i) {
            velocities[i] += particleData->dt * accels[i];
        }

        // Update positions
        for (uint32_t i = particleData->startIdx; i < particleData->endIdx; ++i) {
            positions[i] += particleData->dt * velocities[i];
        }

        const float lifetimeDecrement = particleData->dt * 1000;
        for (uint32_t i = particleData->startIdx; i < particleData->endIdx; ++i) {
            lifetimes[i] -= lifetimeDecrement;
        }
        #endif

        particleData->complete = true;
    }
    
    void JEPhysicsManager::UpdateParticleSystems(std::vector<JEParticleSystem>& particleSystems) {
        JE_TIME currentTime = std::chrono::high_resolution_clock::now();
        const uint32_t elapsedMillis = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_startTime).count();
        
        if (elapsedMillis >= m_updateRateMillis) {
            m_startTime = currentTime;

            // Update particles
            for (uint32_t j = 0; j < particleSystems.size(); ++j) {
                JEParticleSystem& particleSystem = particleSystems[j];

                std::vector<glm::vec3>& positions = particleSystem.m_positionData;
                std::vector<glm::vec3>& velocities = particleSystem.m_velocityData;
                std::vector<glm::vec3>& accels = particleSystem.m_accelData;
                std::vector<float>& lifetimes = particleSystem.m_lifetimeData;

                constexpr bool multithread = true;

                if constexpr (multithread) {
                    const uint32_t numParticlesPerGroup = 10000;
                    const uint32_t numGroups = particleSystem.m_settings.numParticles / numParticlesPerGroup;
                    
                    std::vector<ParticleUpdateData> particleUpdateDataList;
                    particleUpdateDataList.reserve(numGroups);
                    for (uint32_t i = 0; i < numGroups; ++i) {
                        ParticleUpdateData particleUpdate;
                        particleUpdate.complete = false;
                        particleUpdate.dt = m_updateDt;
                        particleUpdate.startIdx = i * numParticlesPerGroup;
                        particleUpdate.endIdx = particleUpdate.startIdx + numParticlesPerGroup;
                        particleUpdate.particleSystem = &particleSystem;
                        particleUpdateDataList.push_back(particleUpdate);
                        JEThreadPoolInstance.EnqueueJob({ UpdateParticleSystems_MT, particleUpdateDataList.data() + i });
                    }

                    // Integrate any remaining particles on this thread
                    for (uint32_t i = numParticlesPerGroup * numGroups; i < particleSystem.m_settings.numParticles; ++i) {
                        velocities[i] += accels[i] * m_updateDt;
                    }

                    for (uint32_t i = numParticlesPerGroup * numGroups; i < particleSystem.m_settings.numParticles; ++i) {
                        positions[i] += velocities[i] * m_updateDt;
                    }

                    for (uint32_t i = numParticlesPerGroup * numGroups; i < particleSystem.m_settings.numParticles; ++i) {
                        lifetimes[i] -= m_updateRateMillis;
                    }

                    // Busy-wait for the thread jobs to complete
                    {
                        //ScopedTimer<float> timer("Busy-wait for particle update threads to complete");
                        while (true) {
                            uint32_t numJobsComplete = 0;
                            for (uint32_t i = 0; i < particleUpdateDataList.size(); ++i) {
                                // If any job is not yet complete, start waiting again
                                if (particleUpdateDataList[i].complete) {
                                    ++numJobsComplete;
                                }
                            }

                            // All jobs completed
                            if (numJobsComplete == particleUpdateDataList.size()) {
                                break;
                            }
                        }
                    }
                } else {
                    #ifdef JOE_ENGINE_SIMD_AVX
                    // TODO: AVX implementation for particle updates
                    #endif

                    #ifdef JOE_ENGINE_SIMD_AVX2
                    // Use AVX2 SIMD commands to integrate multiple particles at a time
                    const uint8_t groupSize = 2;
                    uint32_t numGroups = particleSystem.m_settings.numParticles / groupSize;
                    for (uint32_t i = 0; i < numGroups; ++i) {
                        uint32_t offset = i * groupSize;
                        // Create vector for dt float
                        __m256 dtData = _mm256_set1_ps(m_updateDt);

                        // Copy velocity and acceleration data into registers
                        __m256 velData = _mm256_setr_ps(velocities[offset].x, velocities[offset].y, velocities[offset].z,
                            velocities[offset + 1].x, velocities[offset + 1].y, velocities[offset + 1].z, 0.0f, 0.0f);
                        __m256 accelData = _mm256_setr_ps(accels[offset].x, accels[offset].y, accels[offset].z,
                            accels[offset + 1].x, accels[offset + 1].y, accels[offset + 1].z, 0.0f, 0.0f);

                        // Scale acceleration by dt
                        __m256 accelDt = _mm256_mul_ps(dtData, accelData);
                        // Add result to velocity
                        __m256 velDataUpdated = _mm256_add_ps(accelDt, velData);

                        // Copy position data
                        __m256 posData = _mm256_setr_ps(positions[offset].x, positions[offset].y, positions[offset].z,
                            positions[offset + 1].x, positions[offset + 1].y, positions[offset + 1].z, 0.0f, 0.0f);
                        // Scale velocity by dt
                        __m256 velDt = _mm256_mul_ps(dtData, velDataUpdated);
                        // Add result to position
                        __m256 posDataUpdated = _mm256_add_ps(velDt, posData);

                        // Copy results back to particle system
                        float *velDataPtr = (float*)&velDataUpdated;
                        velocities[offset].x = velDataPtr[0];
                        velocities[offset].y = velDataPtr[1];
                        velocities[offset].z = velDataPtr[2];
                        velocities[offset + 1].x = velDataPtr[3];
                        velocities[offset + 1].y = velDataPtr[4];
                        velocities[offset + 1].z = velDataPtr[5];

                        float *posDataPtr = (float*)&posDataUpdated;
                        positions[offset].x = posDataPtr[0];
                        positions[offset].y = posDataPtr[1];
                        positions[offset].z = posDataPtr[2];
                        positions[offset + 1].x = posDataPtr[3];
                        positions[offset + 1].y = posDataPtr[4];
                        positions[offset + 1].z = posDataPtr[5];
                    }

                    // Integrate leftover particles
                    if (particleSystem.m_settings.numParticles % groupSize != 0) {
                        uint32_t i = numGroups * groupSize;
                        for (; i < particleSystem.m_settings.numParticles; ++i) {
                            velocities[i] += accels[i] * m_updateDt;
                            positions[i] += velocities[i] * m_updateDt;
                        }
                    }

                    // Update particle lifetimes
                    // TODO: make this use SIMD
                    std::vector<float>& lifetimes = particleSystem.m_lifetimeData;
                    for (uint32_t i = 0; i < particleSystem.m_settings.numParticles; ++i) {
                        lifetimes[i] -= m_updateRateMillis;
                    }
                    #endif

                    #ifdef JOE_ENGINE_SIMD_NONE
                    // Update velocities
                    for (uint32_t i = 0; i < particleSystem.m_settings.numParticles; ++i) {
                        velocities[i] += accels[i] * m_updateDt;
                    }

                    // Update positions
                    for (uint32_t i = 0; i < particleSystem.m_settings.numParticles; ++i) {
                        positions[i] += velocities[i] * m_updateDt;
                    }

                    // Update particle lifetimes
                    std::vector<float>& lifetimes = particleSystem.m_lifetimeData;
                    for (uint32_t i = 0; i < particleSystem.m_settings.numParticles; ++i) {
                        lifetimes[i] -= m_updateRateMillis;
                    }
                    #endif
                }
            }
        }
    }
}
