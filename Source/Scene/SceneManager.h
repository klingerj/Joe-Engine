#pragma once

#include "Camera.h"
#include "../Io/IOHandler.h"

namespace JoeEngine {
    class JEEngineInstance;

    class JESceneManager {
    private:
        // Camera(s)
        
        float m_camTranslateSensitivity, m_camRotateSensitivity;

        

        // Scene IDs
        uint32_t m_currentScene;

        JEEngineInstance* m_engineInstance;

    public:
        JESceneManager() : m_camTranslateSensitivity(0.25f), m_camRotateSensitivity(0.05f), m_currentScene(0), m_engineInstance(nullptr) {}
        ~JESceneManager() {}

        // TODO: make these private again
        JECamera m_camera;
        JECamera m_shadowCamera;

        // Creation
        void Initialize(JEEngineInstance* engineInstance);
        void LoadScene(uint32_t sceneId, VkExtent2D windowExtent, VkExtent2D shadowPassExtent);
        void RecreateResources(VkExtent2D windowExtent);

        // IO
        void RegisterCallbacks(JEIOHandler* ioHandler);
    };
}
