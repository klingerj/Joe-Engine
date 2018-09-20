#pragma once

#include <string>

#include "rendering/VulkanRenderer.h"

class EngineApplication {
private:
    // Subsystems
    VulkanRenderer vulkanRenderer;

    // Construct resources/subsystems
    void InitializeEngine();

public:

    // Initialize
    EngineApplication() : vulkanRenderer() {
        InitializeEngine();
    }

    // Destroy resources/subsystems
    ~EngineApplication() {}

    // Run the main loop
    void Run();
};
