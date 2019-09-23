#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

namespace JoeEngine {
    class TransformComponent {
    private:
        glm::vec3 m_translation;
        glm::quat m_rotation;
        glm::vec3 m_scale;
        glm::mat4 m_cachedTransform;

    public:
        friend class JEMeshComponentManager;

        TransformComponent() : m_translation(0, 0, 0), m_rotation(glm::angleAxis(0.0f, glm::vec3(0, 1, 0))), m_scale(1, 1, 1), m_cachedTransform(1.0f) {}
        ~TransformComponent() {}

        void RecomputeTransform() {
            glm::mat4 mat = glm::toMat4(m_rotation);
            m_cachedTransform = glm::translate(glm::mat4(1.0f), m_translation) * glm::toMat4(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);
        }

        // Getters
        const glm::mat4 GetTransform() const {
            return m_cachedTransform;
        }

        const glm::vec3& GetTranslation() const {
            return m_translation;
        }
        
        const glm::quat& GetRotation() const {
            return m_rotation;
        }
        
        const glm::vec3& GetScale() const {
            return m_scale;
        }

        // Setters
        void SetTranslation(const glm::vec3& t) {
            m_translation = t;
            RecomputeTransform();
        }

        void SetRotation(const glm::quat& r) {
            m_rotation = r;
            RecomputeTransform();
        }

        void SetRotation(const float angle, const glm::vec3& axis) {
            m_rotation = glm::angleAxis(angle, axis);
            RecomputeTransform();
        }

        void SetScale(const glm::vec3& s) {
            m_scale = s;
            RecomputeTransform();
        }
    };
}
