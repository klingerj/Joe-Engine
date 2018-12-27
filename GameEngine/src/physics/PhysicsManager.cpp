#include "PhysicsManager.h"
#include "glm/gtc/epsilon.hpp"
#include "glm/gtx/norm.hpp"

void PhysicsManager::Initialize(const std::shared_ptr<MeshDataManager>& m) {
    meshDataManager = m;
}

void PhysicsManager::Update() {
    const JE_TIMER globalTime = std::chrono::high_resolution_clock::now();
    double currentTimeDelta = std::chrono::duration<double, std::chrono::milliseconds::period>(globalTime - m_startTime).count();
    if (currentTimeDelta > m_updateRateInSeconds) {
        auto& meshPhysicsData = meshDataManager->GetMeshData_Physics();
        for (uint32_t i = 0; i < meshDataManager->GetNumMeshes(); ++i) { // TODO: Process multiple meshes at a time with AVX
            // "Collision" with ground
            /*if (meshPhysicsData.positions[i].y < 0.0f) {
                if (meshPhysicsData.velocities[i].y < 0.0f) {
                    meshPhysicsData.velocities[i] *= -0.8f;
                    meshPhysicsData.positions[i].y = 0.0f;
                }
            }*/
            
            meshPhysicsData.obbs[i].center = meshPhysicsData.positions[i];

            // Integration
            if (!(meshPhysicsData.freezeStates[i] & JE_PHYSICS_FREEZE_POSITION)) {
                /*glm::vec3 nextFrameVel = meshPhysicsData.accelerations[i] * m_updateRateFactor;
                meshPhysicsData.positions[i] += 0.5f * (meshPhysicsData.velocities[i] + nextFrameVel) * m_updateRateFactor;
                meshPhysicsData.velocities[i] += nextFrameVel; // this is rk2? */
                meshPhysicsData.velocities[i] += meshPhysicsData.accelerations[i] * m_updateRateFactor;
                meshPhysicsData.positions[i] += meshPhysicsData.velocities[i] * m_updateRateFactor;
            }

            // Compute forces
            glm::vec3 force = glm::vec3(0.0f, -9.80665f, 0.0f);
            const float mass = 1.0f;

            // Inertia Tensor
            glm::mat3 inertiaTensor_body = glm::mat3(1.0f);
            // Hard coded inertia tensor for rectangular prism
            inertiaTensor_body[0][0] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.y * meshPhysicsData.obbs[i].e.y +
                meshPhysicsData.obbs[i].e.z * meshPhysicsData.obbs[i].e.z);
            inertiaTensor_body[1][1] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.x * meshPhysicsData.obbs[i].e.x +
                meshPhysicsData.obbs[i].e.z * meshPhysicsData.obbs[i].e.z);
            inertiaTensor_body[2][2] = 0.08333333f * mass * (meshPhysicsData.obbs[i].e.x * meshPhysicsData.obbs[i].e.x +
                meshPhysicsData.obbs[i].e.y * meshPhysicsData.obbs[i].e.y);
            const glm::mat3 inertiaTensor_bodyInverse = glm::inverse(inertiaTensor_body);
            const glm::mat3 inertiaTensor_Inverse = meshPhysicsData.rotations[i] * inertiaTensor_bodyInverse * glm::transpose(meshPhysicsData.rotations[i]);

            // Initialize torque
            glm::vec3 torque = glm::vec3(0.0f, 0.0f, 0.0f);/*glm::normalize(glm::vec3(1.0f, 0.0f, 0.0f)) * 0.1f;
            if (m_currentTime > 30000.0f) {
                torque *= -1.0f;
            }*/

            // Compute collisions between each OBB in the scene
            for (uint32_t j = 0; j < meshDataManager->GetNumMeshes(); ++j) {
                if (i != j) {
                    CollisionInfo collisionInfo = SAT(meshPhysicsData.obbs[i], meshPhysicsData.obbs[j], i, j);
                    if (!(glm::length(glm::vec3(collisionInfo.minimumTranslation)) == 0.0f && collisionInfo.minimumTranslation.w == -1.0f)) {
                        const float impulseNum = (-2.0f * glm::dot(meshPhysicsData.velocities[i], glm::vec3(collisionInfo.minimumTranslation)));
                        const glm::vec3 impulseDenom = ((1.0f / meshPhysicsData.masses[i]) * glm::cross(inertiaTensor_Inverse * glm::cross(collisionInfo.point, glm::vec3(collisionInfo.minimumTranslation)), collisionInfo.point));
                        const glm::vec3 impulse = impulseNum / impulseDenom;
                        force += impulse;
                        torque += glm::cross(collisionInfo.point, impulse);
                        break;
                    }
                }
            }

            // Force computation to update acceleration
            glm::vec3 acceleration = force;
            meshPhysicsData.accelerations[i] = acceleration;

            // TODO: compute angular velocity, rotation matrix, etc
            if (!(meshPhysicsData.freezeStates[i] & JE_PHYSICS_FREEZE_ROTATION)) {
                meshPhysicsData.angularMomentums[i] += torque * m_updateRateFactor;
                const glm::vec3 angularVelocity = inertiaTensor_Inverse * meshPhysicsData.angularMomentums[i];
                const float angle = std::sqrt(glm::dot(angularVelocity, angularVelocity)) * m_updateRateFactor;

                // Update model matrices
                glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(meshPhysicsData.positions[i]));
                glm::mat4 rotation = glm::mat4(1.0f);
                if (std::fabsf(angle) > 0.000000953674316f) { // 1/2^(20)
                    rotation = glm::rotate(glm::mat4(meshPhysicsData.rotations[i]), angle, angularVelocity);
                }
                meshPhysicsData.rotations[i] = glm::mat3(rotation);
                meshPhysicsData.obbs[i].u[0] = meshPhysicsData.rotations[i][0]; // TODO: this might be wrong w/ the columns idk
                meshPhysicsData.obbs[i].u[1] = meshPhysicsData.rotations[i][1];
                meshPhysicsData.obbs[i].u[2] = meshPhysicsData.rotations[i][2];
                
                meshDataManager->SetModelMatrix(translation * rotation, i);
            }
        }
        m_currentTime += m_updateRateInSeconds;
    }
}

// Checks for intersection between two oriented bounding boxes
CollisionInfo PhysicsManager::SAT(const OBB& obbA, const OBB& obbB, uint32_t indexA, uint32_t indexB) {
    CollisionInfo collisionInfo = {};
    collisionInfo.minimumTranslation = glm::vec4(0.0f, 0.0f, 0.0f, -1.0f);

    const glm::vec3 obbCenterDiff = obbB.center - obbA.center;
    // List of points that are used in a bounding box for collision detection
    // Each point is a potential point of penetration and therefore collision
    // The points are the corners and edge midpoints of a (1, 1, 1)-scale cube
    constexpr uint32_t numOBBPoints = 8;
    const std::array<glm::vec3, numOBBPoints> obbPoints = { glm::vec3(-1.0f, -1.0f, -1.0f),
                                                            glm::vec3(-1.0f, -1.0f,  1.0f),
                                                            glm::vec3(-1.0f,  1.0f, -1.0f),
                                                            glm::vec3(-1.0f,  1.0f,  1.0f),
                                                            glm::vec3(1.0f, -1.0f, -1.0f),
                                                            glm::vec3(1.0f, -1.0f,  1.0f),
                                                            glm::vec3(1.0f,  1.0f, -1.0f),
                                                            glm::vec3(1.0f,  1.0f,  1.0f) };/* ,
                                                            glm::vec3( 1.0f, -1.0f,  0.0f),
                                                            glm::vec3(-1.0f, -1.0f,  0.0f),
                                                            glm::vec3( 1.0f,  1.0f,  0.0f),
                                                            glm::vec3(-1.0f,  1.0f,  0.0f),
                                                            glm::vec3( 0.0f, -1.0f,  1.0f),
                                                            glm::vec3( 0.0f, -1.0f, -1.0f),
                                                            glm::vec3( 0.0f,  1.0f,  1.0f),
                                                            glm::vec3( 0.0f,  1.0f, -1.0f),
                                                            glm::vec3( 1.0f,  0.0f,  1.0f),
                                                            glm::vec3( 1.0f,  0.0f, -1.0f),
                                                            glm::vec3(-1.0f,  0.0f,  1.0f),
                                                            glm::vec3(-1.0f,  0.0f, -1.0f) };*/
    std::array<glm::vec3, numOBBPoints> obbPointsTransformed_A;
    glm::mat4 translationA = glm::translate(glm::mat4(1.0f), obbA.center);
    glm::mat4 transformA = translationA * glm::mat4(glm::vec4(obbA.u[0], 0.0f), glm::vec4(obbA.u[1], 1.0f), glm::vec4(obbA.u[2], 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    for (uint32_t i = 0; i < numOBBPoints; ++i) {
        obbPointsTransformed_A[i] = glm::vec3(meshDataManager->GetModelMatrix(indexA) * glm::vec4(obbPoints[i] * obbA.e, 1.0f));
    }
    std::array<glm::vec3, numOBBPoints> obbPointsTransformed_B;
    glm::mat4 translationB = glm::translate(glm::mat4(1.0f), obbB.center);
    glm::mat4 transformB = translationB * glm::mat4(glm::vec4(obbB.u[0], 0.0f), glm::vec4(obbB.u[1], 1.0f), glm::vec4(obbB.u[2], 1.0f), glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));
    for (uint32_t i = 0; i < numOBBPoints; ++i) {
        obbPointsTransformed_B[i] = glm::vec3(meshDataManager->GetModelMatrix(indexB) * glm::vec4(obbPoints[i] * obbB.e, 1.0f));
    }

    // Now, each array is populated with each obb's world space collision points
    // Now, we test the 15 required axes for the 3D OBB SAT
    std::array<glm::vec3, 15> planeNormals;
    for (uint32_t i = 0; i < 3; ++i) { planeNormals[i] = obbA.u[i]; };
    for (uint32_t i = 0; i < 3; ++i) { planeNormals[i + 3] = obbB.u[i]; };

    // Attempt to cross product the axes. If the cross product cannot be computed i.e. the vectors are identical, just reuse some OBB local vector
    /*planeNormals[6]  = glm::epsilonEqual(obbA.u[0], obbB.u[0], glm::vec3(0.000000953674316f)) ? obbA.u[0] : glm::normalize(glm::cross(obbA.u[0], obbB.u[0]));
    planeNormals[7]  = glm::normalize(glm::cross(obbA.u[0], obbB.u[1]));
    planeNormals[8]  = glm::normalize(glm::cross(obbA.u[0], obbB.u[2]));
    planeNormals[9]  = glm::normalize(glm::cross(obbA.u[1], obbB.u[0]));
    planeNormals[10] = glm::normalize(glm::cross(obbA.u[1], obbB.u[1]));
    planeNormals[11] = glm::normalize(glm::cross(obbA.u[1], obbB.u[2]));
    planeNormals[12] = glm::normalize(glm::cross(obbA.u[2], obbB.u[0]));
    planeNormals[13] = glm::normalize(glm::cross(obbA.u[2], obbB.u[1]));
    planeNormals[14] = glm::normalize(glm::cross(obbA.u[2], obbB.u[2]));*/
    for (uint32_t i = 0; i < 3; ++i) {
        for (uint32_t j = 0; j < 3; ++j) {
            const glm::vec3 vecsEqual = glm::epsilonEqual(obbA.u[0], obbB.u[0], glm::vec3(0.000000953674316f));
            const uint32_t equal = (uint32_t)vecsEqual[0] + (uint32_t)vecsEqual[1] + (uint32_t)vecsEqual[2];
            planeNormals[i * 3 + j + 6] = (equal == 3) ? obbA.u[j] : glm::normalize(glm::cross(obbA.u[i], obbB.u[j]));
        }
    }

    for (uint32_t i = 0; i < 15; ++i) {
        const glm::vec3& planeNormal = planeNormals[i];
        const float diffProj = glm::dot(obbCenterDiff, planeNormal);

        // First, find the most distant point along the normal for each OBB and project it onto the axis
        float maxProjA, maxProjB = -99999999.0f;
        for (uint32_t j = 0; j < numOBBPoints; ++j) {
            const float pointProjA = glm::dot(planeNormal, obbPointsTransformed_A[j] - obbA.center);
            const float pointProjB = glm::dot(planeNormal, obbPointsTransformed_B[j] - obbB.center);
            maxProjA = std::max(maxProjA, std::fabsf(pointProjA));
            maxProjB = std::max(maxProjB, std::fabsf(pointProjB));
            if ((pointProjA < 0.0f) && (pointProjA > collisionInfo.minimumTranslation.w)) { // check if pointProjA penetrates into OBB B at all
                collisionInfo.minimumTranslation = glm::vec4(planeNormal, pointProjA);
                collisionInfo.point = obbPointsTransformed_A[j] - obbA.center;
            }
        }

        // TODO
        // trying to do too much need to:
        // 1. get most radial point and use the max projection to determine if the obbs are even intersecting
        // if yes, iterate over all the points AGAIN and store the direction of minimum penetration and the amount.
        // This is where we will also decide whether or not to store multiple contact points or not, do that later.
        
        // Check for whether or not the axis is separating
        if (std::fabsf(diffProj) > (maxProjA + maxProjB)) {
            collisionInfo.minimumTranslation = glm::vec4(0.0f, 0.0f, 0.0f, -1.0f);
            collisionInfo.point = glm::vec3(0.0f, 0.0f, 0.0f);
            return collisionInfo;
        }
    }

    // Return the collision result otherwise.
    // The collision result contains the minimum penetration information necessary to apply the impulse response
    // TODO: Maybe come back to this if we find that ground collisions are visually shaky.
    return collisionInfo;
}
