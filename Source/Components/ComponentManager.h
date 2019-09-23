#pragma once

namespace JoeEngine {
    class JEComponentManager {
    public:
        JEComponentManager() {}
        virtual ~JEComponentManager() {}

        virtual void Update() = 0;
        virtual void AddNewComponent() = 0;
    };
}
