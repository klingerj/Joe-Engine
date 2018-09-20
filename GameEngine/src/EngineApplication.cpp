#include "EngineApplication.h"

void EngineApplication::Run() {
    while (!vulkanRenderer.GetWindow().ShouldClose()) {
        vulkanRenderer.GetWindow().PollEvents();
        
    }
}

void EngineApplication::InitializeEngine() {
    // Initialize stuff
}
