#pragma once

#include <string>

#include "scene/SceneManager.h"
#include "rendering/VulkanRenderer.h"

class EngineApplication {
private:
    // Subsystems
    SceneManager sceneManager;
    VulkanRenderer vulkanRenderer;

    double frameStartTime, frameEndTime; // timing for performance analysis
    bool enableFrameCounter;

    // Initialize subsystems
    void InitializeEngine();

public:
    EngineApplication() : enableFrameCounter(true), frameStartTime(0.0f), frameEndTime(0.0f) {
        InitializeEngine();
    }

    // Destroy resources/subsystems
    ~EngineApplication() {}

    // Run the main loop
    void Run();

    // Shutdown engine
    void StopEngine();
};
