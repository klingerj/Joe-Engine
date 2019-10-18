#pragma once

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

#include "../Utils/Common.h"
#include "../Components/Mesh/MeshComponent.h"
#include "../Components/Transform/TransformComponent.h"
#include "../Rendering/MeshBufferManager.h"

namespace JoeEngine {
    class JECamera {
    private:
        glm::vec3 m_eye;
        glm::vec3 m_ref;
        glm::vec3 m_look;
        glm::vec3 m_right;
        glm::vec3 m_up;
        float m_aspect;
        float m_nearPlane, m_farPlane;
        glm::mat4 m_viewMatrix;
        glm::mat4 m_projMatrix;
        glm::mat4 m_viewProjMatrix;
        glm::mat4 m_invViewMatrix;
        glm::mat4 m_invProjMatrix;

        void UpdateView() {
            m_look = glm::normalize(m_ref - m_eye);
            m_right = glm::normalize(glm::cross(m_look, JE_WORLD_UP));
            m_up = glm::normalize(glm::cross(m_right, m_look));

            // Update view and viewProj matrices

            glm::mat4 t = glm::mat4(1.0);
            t[3] = glm::vec4(-m_eye.x, -m_eye.y, -m_eye.z, 1.0);

            glm::mat4 o = glm::mat4(1.0);
            o[0] = glm::vec4(m_right.x, m_up.x, m_look.x, 0.0);
            o[1] = glm::vec4(m_right.y, m_up.y, m_look.y, 0.0);
            o[2] = glm::vec4(m_right.z, m_up.z, m_look.z, 0.0);

            m_viewMatrix = o * t;

            m_viewProjMatrix = m_projMatrix * m_viewMatrix;

            m_invViewMatrix = glm::inverse(m_viewMatrix);
            m_invProjMatrix = glm::inverse(m_projMatrix);
        }

        void UpdateProjection() {
            m_projMatrix = glm::perspectiveLH_ZO(JE_FOVY, m_aspect, m_nearPlane, m_farPlane);
            m_projMatrix[1][1] *= -1.0f;
        }

    public:
        JECamera() : JECamera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, JE_SCENE_VIEW_NEAR_PLANE, JE_SCENE_VIEW_FAR_PLANE) {}
        JECamera(glm::vec3 e, glm::vec3 r, float a, float n, float f) : m_eye(e), m_ref(r), m_aspect(a), m_nearPlane(n), m_farPlane(f),
            m_viewMatrix(1.0f), m_projMatrix(1.0f), m_viewProjMatrix(1.0f), m_invViewMatrix(1.0f), m_invProjMatrix(1.0f) {
            UpdateProjection();
            UpdateView();
        }
        ~JECamera() {}

        // Camera Movement and rotation
        void TranslateAlongLook(float amount) {
            m_ref += m_look * amount;
            m_eye += m_look * amount;
            UpdateView();
        }
        void TranslateAlongRight(float amount) {
            m_ref += m_right * amount;
            m_eye += m_right * amount;
            UpdateView();
        }
        void TranslateAlongUp(float amount) {
            m_ref += m_up * amount;
            m_eye += m_up * amount;
            UpdateView();
        }
        void RotateAboutLook(float amount) {
            m_up = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_look) * glm::vec4(m_up, 1.0f)));
            m_right = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_look) * glm::vec4(m_right, 1.0f)));
            UpdateView();
        }
        void RotateAboutRight(float amount) {
            m_look = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_right) * glm::vec4(m_look, 1.0f)));
            m_up = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_right) * glm::vec4(m_up, 1.0f)));
            m_ref = m_eye + m_look;
            UpdateView();
        }
        void RotateAboutUp(float amount) {
            m_look = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_up) * glm::vec4(m_look, 1.0f)));
            m_right = glm::normalize(glm::vec3(glm::rotate(glm::mat4(1.0f), amount, m_up) * glm::vec4(m_right, 1.0f)));
            m_ref = m_eye + m_look;
            UpdateView();
        }

        void SetAspect(float a) {
            m_aspect = a;
            UpdateProjection();
        }
        void SetEye(glm::vec3 e) {
            m_eye = e;
            UpdateView();
        }

        // Getters
        const glm::mat4& GetView() const {
            return m_viewMatrix;
        }
        const glm::mat4& GetProj() const {
            return m_projMatrix;
        }
        glm::mat4 GetViewProj() const {
            return m_viewProjMatrix;
        }
        const glm::mat4& GetInvView() const {
            return m_invViewMatrix;
        }
        const glm::mat4& GetInvProj() const {
            return m_invProjMatrix;
        }
        glm::mat4 GetOrthoViewProj() const {
            const float coord = 10.0f;
            glm::mat4 proj = glm::orthoLH_ZO(-coord, coord, -coord, coord, m_nearPlane, m_farPlane);
            proj[1][1] *= -1.0f;
            return proj * GetView();
        }
        float GetNearPlane() const {
            return m_nearPlane;
        }
        float GetFarPlane() const {
            return m_farPlane;
        }

        bool Cull(const MeshComponent& meshComponent, const TransformComponent& transformComponent, const BoundingBoxData& boundingBox) const {
            const glm::mat4 transVS = GetViewProj() * transformComponent.GetTransform();
            
            BoundingBoxData bbData;
            //bool inside = false;
            for (uint8_t i = 0; i < boundingBox.size(); ++i) {
                glm::vec4 bbPointCS = transVS * glm::vec4(boundingBox[i], 1.0f);
                bbPointCS /= bbPointCS.w;
                /*inside |= bbPointCS.x > -1.0f &&
                          bbPointCS.y > -1.0f &&
                          bbPointCS.z > 0.0f &&
                          bbPointCS.x < 1.0f &&
                          bbPointCS.y < 1.0f &&
                          bbPointCS.z < 1.0f;*/
                
                int inside = 0;
                inside += (int)(bbPointCS.x > -1.0f);
                inside += (int)(bbPointCS.y > -1.0f);
                inside += (int)(bbPointCS.z > 0.0f);
                inside += (int)(bbPointCS.x < 1.0f);
                inside += (int)(bbPointCS.y < 1.0f);
                inside += (int)(bbPointCS.z < 1.0f);
                if (inside == 6) {
                    return true;
                }

                bbData[i] = bbPointCS;
            }

            // Resolve false negatives (e.g. - large plane where all four corners are outside the view frustum)
            bool intersecting = false;

            const int maxIdx = boundingBox.size() - 1;
            // if the x-coord of the obb's min/max points are on opposite sides of the left view frustum plane
            intersecting |= std::signbit(bbData[0].x + 1.0f) != std::signbit(bbData[maxIdx].x + 1.0f);
            // if the x-coord of the obb's min/max points are on opposite sides of the right view frustum plane
            intersecting |= std::signbit(bbData[0].x - 1.0f) != std::signbit(bbData[maxIdx].x - 1.0f);
            
            // if the y-coord of the obb's min/max points are on opposite sides of the bottom view frustum plane
            intersecting |= std::signbit(bbData[0].y + 1.0f) != std::signbit(bbData[maxIdx].y + 1.0f);
            // if the y-coord of the obb's min/max points are on opposite sides of the top view frustum plane
            intersecting |= std::signbit(bbData[0].y - 1.0f) != std::signbit(bbData[maxIdx].y - 1.0f);
            
            // if the x-coord of the obb's min/max points are on opposite sides of the near clip view frustum plane
            intersecting |= std::signbit(bbData[0].z) != std::signbit(bbData[maxIdx].z);
            // if the x-coord of the obb's min/max points are on opposite sides of the far clip view frustum plane
            intersecting |= std::signbit(bbData[0].z - 1.0f) != std::signbit(bbData[maxIdx].z - 1.0f);
            // Note: depth lies on [0, 1] in this engine!

            if (intersecting) {
                return true;
            }

            // TODO: resolve false positives?

            return false;
        }
    };
}
