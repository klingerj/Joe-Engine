#include "PhysicsManager.h"

void PhysicsManager::Initialize(std::shared_ptr<MeshDataManager> m) {
    meshDataManager = m;
}

void PhysicsManager::Update() {
    const MeshData_Physics& meshPhysicsData = meshDataManager->GetMeshData_Physics();
    for (int i = 0; i < NUM_MESHES; ++i) {
        // TODO: get forces + compute and integrate accel/vel/pos
        // Fake collisions for now
        // compute rotations
        // set model matrices
    }
}
