#pragma once

#include "../Utils/Common.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

namespace JoeEngine {
    class JECamera {
    private:
        glm::vec3 m_eye;
        glm::vec3 m_ref;
        glm::vec3 m_look;
        glm::vec3 m_right;
        glm::vec3 m_up;
        float m_aspect;
        float m_nearPlane, farPlane;

    public:
        JECamera() : JECamera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE) {}
        JECamera(glm::vec3 e, glm::vec3 r, float a, float n, float f) : m_eye(e), m_ref(r), m_aspect(a), m_nearPlane(n), farPlane(f) {
            ComputeAttributes();
        }
        ~JECamera() {}

        void ComputeAttributes() {
            m_look = glm::normalize(m_ref - m_eye);
            m_right = glm::normalize(glm::cross(m_look, JE_WORLD_UP));
            m_up = glm::normalize(glm::cross(m_right, m_look));
        }

        // Camera Movement and rotation
        void TranslateAlongLook(float amount) {
            m_ref += m_look * amount;
            m_eye += m_look * amount;
        }
        void TranslateAlongRight(float amount) {
            m_ref += m_right * amount;
            m_eye += m_right * amount;
        }
        void TranslateAlongUp(float amount) {
            m_ref += m_up * amount;
            m_eye += m_up * amount;
        }
        void RotateAboutLook(float amount) {
            m_up = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_look) * glm::vec4(m_up, 1.0f)));
            m_right = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_look) * glm::vec4(m_right, 1.0f)));
        }
        void RotateAboutRight(float amount) {
            m_look = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_right) * glm::vec4(m_look, 1.0f)));
            m_up = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_right) * glm::vec4(m_up, 1.0f)));
            m_ref = m_eye + m_look;
        }
        void RotateAboutUp(float amount) {
            m_look = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_up) * glm::vec4(m_look, 1.0f)));
            m_right = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_up) * glm::vec4(m_right, 1.0f)));
            m_ref = m_eye + m_look;
        }

        void SetAspect(float a) {
            m_aspect = a;
        }
        void SetEye(glm::vec3 e) {
            m_eye = e;
            ComputeAttributes();
        }

        // Getters
        glm::mat4 GetView() const {
            return glm::lookAt(m_eye, m_ref, JE_WORLD_UP);
        }
        glm::mat4 GetProj() const {
            return glm::perspective(JE_FOVY, m_aspect, m_nearPlane, farPlane);
        }
        float GetNearPlane() const {
            return m_nearPlane;
        }
        float GetFarPlane() const {
            return farPlane;
        }
    };
}
