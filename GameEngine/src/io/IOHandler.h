#pragma once

#include <map>
#include <functional>

#include "../utils/Common.h"
#include "GLFW/glfw3.h"

class IOHandler {
private:
    using CallbackFunction = std::function<void()>;
    GLFWwindow* window;
    std::map<int, std::vector<CallbackFunction>> callbacks;

    void SetupGLFWCallbackFunctions();

public:
    IOHandler() {}
    ~IOHandler() {}

    void Initialize(GLFWwindow* glfwWindow);
    void PollInput();
    void AddCallback(int key, CallbackFunction callback);
    void ExecuteCallbacksForKey(int key);
};
