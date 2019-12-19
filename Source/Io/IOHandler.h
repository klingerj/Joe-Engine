#pragma once

#include <map>

#include "GLFW/glfw3.h"

#include "../Utils/Common.h"

namespace JoeEngine {
    //! The JEIOHandler class
    /*!
      Class that maps keyboard inputs to callback functions.
      \sa JEEngineInstance
    */
    class JEIOHandler {
    private:
        //! GLFW Window.
        /*! Pointer to the GLFW window instance for this application. */
        GLFWwindow* m_window;

        //! Callback function map.
        /*! Maps keyboard input indices (integers) to a list of functions to be called when that key is pressed. */
        std::map<int, std::vector<JECallbackFunction>> m_callbacks;

        //! Setup callback functions.
        /*! Private function called once to initalize the GLFW callback functions for keyboard and mouse events. */
        void SetupGLFWCallbackFunctions();

    public:
        //! Constructor.
        JEIOHandler() = default;

        //! Destructor.
        ~JEIOHandler() = default;

        //! Initialization.
        /*!
          Called once by the JEEngineInstance that owns this class. Sets the member pointer to the specified window.
          \param glfwWindow the specified GLFW window to expect keyboard and mouse events from.
        */
        void Initialize(GLFWwindow* glfwWindow);

        //! Poll for input.
        /*!
          Wrapper function around the GLFW poll input function.
        */
        void PollInput();

        //! Add callback function.
        /*!
          Registers the specified callback function to the specified key index. 
          \param key The keypress to register a clalback function to.
          \param callback the callback function to register
        */
        void AddCallback(int key, JECallbackFunction callback);

        //! Execute callbacks for key.
        /*!
          Invokes all callbacks registered for the specified key index.
          \param key the key index to execute the callbacks of
        */
        void ExecuteCallbacksForKey(int key);
    };
}
