#pragma once

#include <map>

#include "../Utils/Common.h"
#include "GLFW/glfw3.h"

namespace JoeEngine {
    class JEIOHandler {
    private:
        GLFWwindow* m_window;
        std::map<int, std::vector<JECallbackFunction>> m_callbacks;

        void SetupGLFWCallbackFunctions();

    public:
        JEIOHandler() {}
        ~JEIOHandler() {}

        void Initialize(GLFWwindow* glfwWindow);
        void PollInput();
        void AddCallback(int key, JECallbackFunction callback);
        void ExecuteCallbacksForKey(int key);
    };
}
