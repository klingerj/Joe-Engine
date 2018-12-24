#pragma once

#include <chrono>

#include "../scene/MeshDataManager.h"

class PhysicsManager {
private:
    std::shared_ptr<MeshDataManager> meshDataManager;

    using JE_TIMER = std::chrono::time_point<std::chrono::steady_clock>;
    const JE_TIMER m_startTime; // Start time of the physics system
    double m_currentTime; // Current time of the physics system
    const double m_updateRateInSeconds; // in s
    const float m_updateRateFactor; // For physics integration

public:
    PhysicsManager() : m_startTime(std::chrono::high_resolution_clock::now()), m_currentTime(0.0), m_updateRateInSeconds(33.333), m_updateRateFactor(1.0f / 33.333f) {}
    ~PhysicsManager() {}

    void Initialize(const std::shared_ptr<MeshDataManager>& m);

    // Compute physics on the mesh data
    void Update();
};

// Collision information
// TODO: expand this to store info for edge-edge collisions. Right now we are just checking vertex-face.
typedef struct collision_info_t {
    glm::vec4 minimumTranslation; // x, y, & z contains the direction, w contains the minimum penetration distance of the two object collision
    glm::vec4 point; // The point on the bounding box that the collision should be applied to 
} CollisionInfo;
