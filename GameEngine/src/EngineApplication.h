#pragma once

#include <string>

#include "io/IOHandler.h"
#include "scene/SceneManager.h"
#include "rendering/VulkanRenderer.h"
#include "physics/PhysicsManager.h"

class JEEngineApplication {
private:
    // Subsystems
    JESceneManager sceneManager;
    JEVulkanRenderer vulkanRenderer;
    JEIOHandler ioHandler;
    JEPhysicsManager physicsManager;

    double frameStartTime, frameEndTime; // timing for performance analysis
    bool enableFrameCounter;

    // Initialize subsystems
    void InitializeEngine();

public:
    JEEngineApplication() : enableFrameCounter(false), frameStartTime(0.0f), frameEndTime(0.0f) {
        InitializeEngine();
    }

    // Destroy resources/subsystems
    ~JEEngineApplication() {}

    // Run the main loop
    void Run();

    // Shutdown engine
    void StopEngine();

    // Getters
    JEVulkanRenderer* GetRenderSubsystem() {
        return &vulkanRenderer;
    }
    JEIOHandler* GetIOSubsystem() {
        return &ioHandler;
    }
};
