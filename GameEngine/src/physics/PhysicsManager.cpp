#include "PhysicsManager.h"
#include <iostream>
void PhysicsManager::Initialize(const std::shared_ptr<MeshDataManager>& m) {
    meshDataManager = m;
}

void PhysicsManager::Update() {
    const JE_TIMER globalTime = std::chrono::high_resolution_clock::now();
    double currentTimeDelta = std::chrono::duration<double, std::chrono::milliseconds::period>(globalTime - m_startTime).count();
    if (currentTimeDelta > m_updateRateInSeconds) {
        auto& meshPhysicsData = meshDataManager->GetMeshData_Physics();
        for (uint32_t i = 0; i < meshDataManager->GetNumMeshes(); ++i) { // TODO: Process multiple meshes at a time with AVX
            // Collisions
            if (meshPhysicsData.positions[i].y < 0.0f) {
                if (meshPhysicsData.velocities[i].y < 0.0f) {
                    meshPhysicsData.velocities[i] *= -0.8f;
                    meshPhysicsData.positions[i].y = 0.0f;
                }
            }

            // Integration
            if (!(meshPhysicsData.freezeStates[i] & JE_PHYSICS_FREEZE_POSITION)) {
                glm::vec3 nextFrameVel = meshPhysicsData.accelerations[i] * m_updateRateFactor;
                meshPhysicsData.positions[i] += 0.5f * (meshPhysicsData.velocities[i] + nextFrameVel) * m_updateRateFactor;
                meshPhysicsData.velocities[i] += nextFrameVel;
            }

            // Compute forces
            const glm::vec3 force_gravity = glm::vec3(0.0f, -9.80665f, 0.0f);
            const float mass = 1.0f;

            // Force computation to update acceleration
            glm::vec3 acceleration = force_gravity;
            meshPhysicsData.accelerations[i] = acceleration;

            // TODO: compute angular velocity, rotation matrix, etc
            if (!(meshPhysicsData.freezeStates[i] & JE_PHYSICS_FREEZE_ROTATION)) {
                glm::mat3 inertiaTensor_body = glm::mat3(1.0f);
                // Hard coded inertia tensor for rectangular prism
                inertiaTensor_body[0][0] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.y * meshPhysicsData.obbs[i].e.y +
                    meshPhysicsData.obbs[i].e.z * meshPhysicsData.obbs[i].e.z);
                inertiaTensor_body[1][1] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.x * meshPhysicsData.obbs[i].e.x +
                    meshPhysicsData.obbs[i].e.z * meshPhysicsData.obbs[i].e.z);
                inertiaTensor_body[2][2] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.x * meshPhysicsData.obbs[i].e.x +
                    meshPhysicsData.obbs[i].e.y * meshPhysicsData.obbs[i].e.y);
                glm::mat3 inertiaTensor_bodyInverse = glm::inverse(inertiaTensor_body);

                glm::vec3 torque = glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * 0.1f;
                if (m_currentTime > 30000.0f) {
                    torque *= -1.0f;
                }
                meshPhysicsData.angularMomentums[i] += torque * m_updateRateFactor;
                glm::mat3 inertiaTensor_Inverse = meshPhysicsData.rotations[i] * inertiaTensor_bodyInverse * glm::transpose(meshPhysicsData.rotations[i]);
                glm::vec3 angularVelocity = inertiaTensor_Inverse * meshPhysicsData.angularMomentums[i];
                float angle = std::sqrt(glm::dot(angularVelocity, angularVelocity)) * m_updateRateFactor;

                // Update model matrices
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(meshPhysicsData.positions[i]));
                glm::mat4 rotation = glm::rotate(glm::mat4(meshPhysicsData.rotations[i]), angle, angularVelocity);
                meshPhysicsData.rotations[i] = glm::mat3(rotation);
                meshDataManager->SetModelMatrix(translation * rotation, i);
            }
        }
        m_currentTime += m_updateRateInSeconds;
    }
}
