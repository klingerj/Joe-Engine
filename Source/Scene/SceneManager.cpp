#include <chrono>

#include "SceneManager.h"
#include "../EngineInstance.h"
#include "../Components/Rotator/RotatorComponentManager.h"

namespace JoeEngine {
    void JESceneManager::Initialize(JEEngineInstance* engineInstance) {
        m_engineInstance = engineInstance;
    }

    void JESceneManager::LoadScene(uint32_t sceneId, VkExtent2D windowExtent, VkExtent2D shadowPassExtent) {
        m_currentScene = sceneId;
        if (sceneId == 0) {
            std::vector<Entity> entities;
            MeshComponent meshComp_wahoo = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "wahoo.obj");
            for (int i = 0; i < 64; ++i) {
                for (int j = 0; j < 64; ++j) {
                    Entity newEntity = m_engineInstance->SpawnEntity();
                    entities.push_back(newEntity);
                    m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_wahoo);
                    
                    TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
                    trans->SetTranslation(glm::vec3(i - 32, 0, j - 32) * 0.1f);
                    trans->SetScale(glm::vec3(0.01f, 0.01f, 0.01f));

                    m_engineInstance->AddComponent<RotatorComponent>(newEntity);
                    RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
                    rot->m_entityId = newEntity.GetId();
                    rot->m_axis = glm::vec3(0, 1, 0);
                }
            }
            
            Entity newEntity = m_engineInstance->SpawnEntity();
            entities.push_back(newEntity);
            MeshComponent meshComp_plane = m_engineInstance->CreateMeshComponent(JE_MODELS_OBJ_DIR + "plane.obj");
            m_engineInstance->SetComponent<JEMeshComponentManager>(newEntity, meshComp_plane);
            TransformComponent* trans = m_engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(newEntity);
            trans->SetTranslation(glm::vec3(0.0f, -0.25f, 0.0f));
            trans->SetRotation(glm::angleAxis(glm::radians(-90.0f), glm::vec3(1, 0, 0)));
            trans->SetScale(glm::vec3(25.0f, 25.0f, 1.0f));
            m_engineInstance->AddComponent<RotatorComponent>(newEntity);
            RotatorComponent* rot = m_engineInstance->GetComponent<RotatorComponent, RotatorComponentManager>(newEntity);
            rot->m_entityId = newEntity.GetId();
            
            // Camera
            m_camera = JECamera(glm::vec3(0.0f, 4.0f, 12.0f), glm::vec3(0.0f, 0.0f, 0.0f), windowExtent.width / (float)windowExtent.height, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE);
            m_shadowCamera = JECamera(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), shadowPassExtent.width / (float)shadowPassExtent.height, JE_SHADOW_VIEW_NEAR_PLANE, JE_SHADOW_VIEW_FAR_PLANE);
        } else if (sceneId == 1) {

        }
    }

    void JESceneManager::RecreateResources(VkExtent2D windowExtent) {
        m_camera.SetAspect(windowExtent.width / (float)windowExtent.height);
    }

    void JESceneManager::RegisterCallbacks(JEIOHandler* ioHandler) {
        // Camera Movement
        JECallbackFunction cameraPanForward = [&] { m_camera.TranslateAlongLook(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanBackward = [&] { m_camera.TranslateAlongLook(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPanLeft = [&] { m_camera.TranslateAlongRight(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPanRight = [&] { m_camera.TranslateAlongRight(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanUp = [&] { m_camera.TranslateAlongUp(m_camTranslateSensitivity); };
        JECallbackFunction cameraPanDown = [&] { m_camera.TranslateAlongUp(-m_camTranslateSensitivity); };
        JECallbackFunction cameraPitchDown = [&] { m_camera.RotateAboutRight(-m_camRotateSensitivity); };
        JECallbackFunction cameraPitchUp = [&] { m_camera.RotateAboutRight(m_camRotateSensitivity); };
        JECallbackFunction cameraYawLeft = [&] { m_camera.RotateAboutUp(-m_camRotateSensitivity); };
        JECallbackFunction cameraYawRight = [&] { m_camera.RotateAboutUp(m_camRotateSensitivity); };
        ioHandler->AddCallback(JE_KEY_W, cameraPanForward);
        ioHandler->AddCallback(JE_KEY_A, cameraPanLeft);
        ioHandler->AddCallback(JE_KEY_S, cameraPanBackward);
        ioHandler->AddCallback(JE_KEY_D, cameraPanRight);
        ioHandler->AddCallback(JE_KEY_Q, cameraPanDown);
        ioHandler->AddCallback(JE_KEY_E, cameraPanUp);
        ioHandler->AddCallback(JE_KEY_UP, cameraPitchDown);
        ioHandler->AddCallback(JE_KEY_LEFT, cameraYawRight);
        ioHandler->AddCallback(JE_KEY_DOWN, cameraPitchUp);
        ioHandler->AddCallback(JE_KEY_RIGHT, cameraYawLeft);
    }
}
