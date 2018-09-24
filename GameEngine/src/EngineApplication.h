#pragma once

#include <string>

#include "rendering/VulkanRenderer.h"

class EngineApplication {
private:
    // Vulkan Device wrapper class for managing Vulkan application resources
    // Listed first, so it will be constructed first and will destruct last
    VulkanDevice vulkanDevice;

    // Subsystems
    VulkanRenderer vulkanRenderer;

    // Construct resources/subsystems
    void InitializeEngine();

public:
    // Initialize
    EngineApplication() : vulkanDevice(), vulkanRenderer(vulkanDevice) {
        //InitializeEngine();
    }

    // Destroy resources/subsystems
    ~EngineApplication() {}

    // Run the main loop
    void Run();
};
