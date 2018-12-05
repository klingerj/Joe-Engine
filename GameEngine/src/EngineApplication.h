#pragma once

#include <string>

#include "io/IOHandler.h"
#include "scene/SceneManager.h"
#include "rendering/VulkanRenderer.h"
#include "physics/PhysicsManager.h"

class EngineApplication {
private:
    // Subsystems
    SceneManager sceneManager;
    VulkanRenderer vulkanRenderer;
    IOHandler ioHandler;
    PhysicsManager physicsManager;

    double frameStartTime, frameEndTime; // timing for performance analysis
    bool enableFrameCounter;

    // Initialize subsystems
    void InitializeEngine();

public:
    EngineApplication() : enableFrameCounter(false), frameStartTime(0.0f), frameEndTime(0.0f) {
        InitializeEngine();
    }

    // Destroy resources/subsystems
    ~EngineApplication() {}

    // Run the main loop
    void Run();

    // Shutdown engine
    void StopEngine();

    // Getters
    VulkanRenderer* GetRenderSubsystem() {
        return &vulkanRenderer;
    }
    IOHandler* GetIOSubsystem() {
        return &ioHandler;
    }
};
