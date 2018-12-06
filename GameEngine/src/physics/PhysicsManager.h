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
    PhysicsManager() : m_startTime(std::chrono::high_resolution_clock::now()), m_currentTime(0.0), m_updateRateInSeconds(33.333), m_updateRateFactor(1.0f / (float)m_updateRateInSeconds) {}
    ~PhysicsManager() {}

    void Initialize(std::shared_ptr<MeshDataManager> m);

    // Compute physics on the mesh data
    void Update();
};
