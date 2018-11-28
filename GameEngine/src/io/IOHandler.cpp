#include "IOHandler.h"
#include "../rendering/VulkanWindow.h"
#include "../EngineApplication.h"

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto io = reinterpret_cast<EngineApplication*>(glfwGetWindowUserPointer(window))->GetIOSubsystem();

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        io->ExecuteCallbacksForKey(key);
    }
}

static void mouse_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        // stuff
    }
}

void IOHandler::SetupGLFWCallbackFunctions() {
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);
}

void IOHandler::Initialize(GLFWwindow* glfwWindow) {
    window = glfwWindow;
    SetupGLFWCallbackFunctions();
}

void IOHandler::PollInput() {
    glfwPollEvents();
}

void IOHandler::AddCallback(int key, CallbackFunction callback) {
    callbacks[key].push_back(callback);
}

void IOHandler::ExecuteCallbacksForKey(int key) {
    for (CallbackFunction f : callbacks[key]) {
        f();
    }
}
