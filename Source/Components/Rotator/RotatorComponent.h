#pragma once

class TransformComponent;

namespace JoeEngine {
    class RotatorComponent {
    public:
        RotatorComponent() : transform(nullptr) {}
        ~RotatorComponent() {}
        
        TransformComponent* transform;
        
        void Update();
    };
}
