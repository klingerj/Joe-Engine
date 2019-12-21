#pragma once

#include "Camera.h"
#include "../Io/IOHandler.h"

namespace JoeEngine {
    class JEEngineInstance;

    //! The Scene Manager class.
    /*!
      Class that manages all data for scene management. Currently owns camera data (won't in the future). The most important function
      is LoadScene(), which makes all necessary engine calls to create a scene and is where user scene editing occurs programmatically.
    */
    // TODO: If scene creation is moved from programmatic to file loading, this class would probably not be necessary,
    // or would change to a class (or namespace) would read and parse inputs and make the corresponding engine API calls.
    class JESceneManager {
    private:
        //! Camera movement sensitivity.
        float m_camTranslateSensitivity;

        //! Camera rotation sensitivity.
        float m_camRotateSensitivity;

        //! Currently loaded scene ID.
        uint32_t m_currentScene;

        //! Reference to the engine instance for making API calls.
        JEEngineInstance* m_engineInstance;

    public:
        //! Default constructor.
        /*! Initializes member variables. Engine later invokes Initialize(). */
        JESceneManager() : m_camTranslateSensitivity(0.25f), m_camRotateSensitivity(0.05f), m_currentScene(0), m_engineInstance(nullptr) {}

        //! Destructor (default).
        ~JESceneManager() = default;

        // TODO: make these private
        //! Main scene rendering camera.
        JECamera m_camera;

        //! Light source / shadow map camera.
        JECamera m_shadowCamera;

        //! Initialization function.
        void Initialize(JEEngineInstance* engineInstance);

        //! Load the specified scene.
        /*!
          Loads the scene specified by sceneId.
          \param sceneId the id of the scene to load
          \param windowExtent the extent for the scene rendering camera
          \param shadowPassExtent the extent for the light source / shadow map rendering camera
        */
        void LoadScene(uint32_t sceneId, VkExtent2D windowExtent, VkExtent2D shadowPassExtent);

        //! Update camera resources.
        /*!
          Updates scene resources. Currently just updates the aspect ratio of the scene rendering camera.
          \param windowExtent the new extent for the scene rendering camera.
        */
        void RecreateResources(VkExtent2D windowExtent);

        //! Register callback functions with IOHandler.
        /*!
          Links custom functions to keypresses with the IOHandler.
          \param ioHandler the ioHandler subsystem to register callback functions with.
        */
        void RegisterCallbacks(JEIOHandler* ioHandler);
    };
}
