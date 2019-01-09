#include "IOHandler.h"
#include "../rendering/VulkanWindow.h"
#include "../EngineApplication.h"

namespace JoeEngine {
    static void JEKey_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        auto io = reinterpret_cast<JEEngineApplication*>(glfwGetWindowUserPointer(window))->GetIOSubsystem();

        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            io->ExecuteCallbacksForKey(key);
        }
    }

    static void JEMouse_callback(GLFWwindow* window, int button, int action, int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            // stuff
        }
    }

    void JEIOHandler::SetupGLFWCallbackFunctions() {
        glfwSetKeyCallback(m_window, JEKey_callback);
        glfwSetMouseButtonCallback(m_window, JEMouse_callback);
    }

    void JEIOHandler::Initialize(GLFWwindow* glfwWindow) {
        m_window = glfwWindow;
        SetupGLFWCallbackFunctions();
    }

    void JEIOHandler::PollInput() {
        glfwPollEvents();
    }

    void JEIOHandler::AddCallback(int key, JECallbackFunction callback) {
        m_callbacks[key].push_back(callback);
    }

    void JEIOHandler::ExecuteCallbacksForKey(int key) {
        for (JECallbackFunction f : m_callbacks[key]) {
            f();
        }
    }
}
