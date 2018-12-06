#include "PhysicsManager.h"

void PhysicsManager::Initialize(std::shared_ptr<MeshDataManager> m) {
    meshDataManager = m;
}

void PhysicsManager::Update() {
    const JE_TIMER globalTime = std::chrono::high_resolution_clock::now();
    double currentTimeDelta = std::chrono::duration<double, std::chrono::milliseconds::period>(globalTime - m_startTime).count();
    if (currentTimeDelta > m_updateRateInSeconds) {
        auto& meshPhysicsData = meshDataManager->GetMeshData_Physics();
        for (uint32_t i = 0; i < meshDataManager->GetNumMeshes(); ++i) { // TODO: Process multiple meshes at a time with AVX
            // Euler integration
            // TODO: RK2
            if (!(meshPhysicsData.freezeStates[i] & JE_PHYSICS_FREEZE_POSITION)) {
                meshPhysicsData.positions[i] += meshPhysicsData.velocities[i] * m_updateRateFactor;
                meshPhysicsData.velocities[i] += meshPhysicsData.accelerations[i] * m_updateRateFactor;
            }

            // Collisions
            if (meshPhysicsData.positions[i].y < 0.0f) { // Collide with an invisible plane at y = -1
                meshPhysicsData.velocities[i] *= -0.9f;
                //collisionForce = glm::vec3(0.0f, 9.80665f, 0.0f);
            }

            // Compute forces
            const glm::vec3 force_gravity = glm::vec3(0.0f, -9.80665f, 0.0f);
            const float mass = 1.0f;

            // Force computation to update acceleration
            glm::vec3 acceleration = force_gravity / mass;
            meshPhysicsData.accelerations[i] = acceleration;

            // TODO: compute angular velocity, rotation matrix, etc


            // Update model matrices
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(meshPhysicsData.positions[i]));
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), 0.0f, glm::vec3(0.0f, 1.0f, 0.0f));
            meshDataManager->SetModelMatrix(translation * rotation, i);
        }
        m_currentTime += m_updateRateInSeconds;
    }
}
