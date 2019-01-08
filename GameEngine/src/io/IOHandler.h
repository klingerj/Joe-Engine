#pragma once

#include <map>

#include "../utils/Common.h"
#include "GLFW/glfw3.h"

class JEIOHandler {
private:
    GLFWwindow* window;
    std::map<int, std::vector<JECallbackFunction>> callbacks;

    void SetupGLFWCallbackFunctions();

public:
    JEIOHandler() {}
    ~JEIOHandler() {}

    void Initialize(GLFWwindow* glfwWindow);
    void PollInput();
    void AddCallback(int key, JECallbackFunction callback);
    void ExecuteCallbacksForKey(int key);
};
