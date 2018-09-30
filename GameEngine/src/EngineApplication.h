#pragma once

#include <string>

#include "rendering/VulkanRenderer.h"

class EngineApplication {
private:
    // Subsystems
    VulkanRenderer vulkanRenderer;

    // Initialize subsystems
    void InitializeEngine();

public:
    EngineApplication() {
        InitializeEngine();
    }

    // Destroy resources/subsystems
    ~EngineApplication() {}

    // Run the main loop
    void Run();

    // Shutdown engine
    void StopEngine();
};
