#pragma once

#include "../scene/MeshDataManager.h"

class PhysicsManager {
private:
    std::shared_ptr<MeshDataManager> meshDataManager;

public:
    PhysicsManager() {}
    ~PhysicsManager() {}

    void Initialize(std::shared_ptr<MeshDataManager> m);

    // Compute physics on the mesh data
    void Update();
};