#pragma once

#include <string>
#include "../Utils/Common.h"

namespace JoeEngine {
    class JEVulkanRenderer;

    //! The JEVulkanWindow class.
    /*!
      Wrapper class with convenience functions for the GLFW window used by the engine and the Vulkan surface object.
    */
    class JEVulkanWindow {
    private:
        //! The GLFW window used by the engine.
        GLFWwindow* m_window;

        //! The Vulkan surface object.
        VkSurfaceKHR m_surface;

    public:
        //! Constructor (default).
        JEVulkanWindow() = default;

        //! Destructor (default).
        ~JEVulkanWindow() = default;

        //! Initialization.
        /*!
          Makes crucial GLFW initialization calls.
          \param w window width.
          \param h window height.
          \param n window name.
        */
        void Initialize(const int w, const int h, const std::string& n);

        //! Cleanup.
        /*!
          Makes crucial GLFW and Vulkan cleanup calls.
          \param instance the Vulkan instance used to cleanup.
        */
        void Cleanup(VkInstance instance);

        //! Create a Vulkan surface
        /*!
          Creates a vulkan surface object from the GLFW window.
          \param instance the Vulkan instance used to create the surface object.
        */
        void SetupVulkanSurface(VkInstance instance);

        //! Get surface.
        /*!
          Returns the surface object.
          \return the surface object.
        */
        VkSurfaceKHR GetSurface() const {
            return m_surface;
        }

        //! Get the GLFW window.
        /*!
          Returns the GLFW window.
          \return the GLFW window.
        */
        GLFWwindow* GetWindow() const {
            return m_window;
        }

        // Somewhat useless wrapping, but makes calls to the equivalent GLFW functions (needed for main game/render loop)
        // GLFW functions wrapped into member functions

        //! Should window close.
        /*!
          Invokes the glfw function checking if the window should close, (e.g. the user clicked the red X to
          close the window).
          \return true if the window should close, false otherwise.
        */
        bool ShouldClose() const {
            return glfwWindowShouldClose(m_window);
        }

        //! Get required instance extensions.
        /*!
          Invokes the glfw function returning the required extensions for the Vulkan instance.
          \return pointer to list of extension names.
          \param count a variable that is set to the number of required instance extensions that are returned.
        */
        const char** GetRequiredInstanceExtensions(uint32_t* count) const {
            return glfwGetRequiredInstanceExtensions(count);
        }

        //! Set framebuffer callback function.
        /*!
          Invokes the glfw function that sets the framebuffer size-change callback function.
          \param framebufferResizeCallback the callback function.
        */
        void SetFrameBufferCallback(GLFWframebuffersizefun framebufferResizeCallback) const {
            glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
        }

        //! Await window maximize.
        /*!
          Makes glfw function calls that allows the application to sleep while minimized.
          This is called in a while loop from the renderer that constantly checks if the width and height parameters
          are equal to 0 (minimized) or not (maximized or otherwise). 
        */
        void AwaitMaximize(int* width, int* height) const {
            glfwGetFramebufferSize(m_window, width, height);
            glfwWaitEvents();
        }
    };
}
