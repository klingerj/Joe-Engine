#pragma once

#include "glm/glm.hpp"
#include "glm/gtx/quaternion.hpp"

namespace JoeEngine {
    //! The Transform Component class
    /*!
      Contains the necessary transformation info to be attached to a particular entity.
      This consists of one each of a translation, rotation, and scale.
      A cached transform matrix is also stored to prevent excessively computing the overall transformation
      matrix from the stored translation, rotation, and scale data.
      \sa JETransformComponentManager, JEVulkanRenderer
    */
    class TransformComponent {
    private:
        //! Translation.
        /*! Vector3 translation data of this component. */
        glm::vec3 m_translation;

        //! Rotation.
        /*! Quaternion rotation data of this component. */
        glm::quat m_rotation;

        //! Scale.
        /*! Vector3 scale data of this component. */
        glm::vec3 m_scale;

        //! Cached transform.
        /*! The overall transform of this component. */
        glm::mat4 m_cachedTransform;

    public:
        //! Default constructor.
        /*!
          Initializes all members to an appropriate transformation.
          Translation - 0, 0, 0
          Rotation - 0
          Scale - 1, 1, 1
          Cached transform - identity matrix
        */
        TransformComponent() : m_translation(0, 0, 0), m_rotation(glm::angleAxis(0.0f, glm::vec3(0, 1, 0))), m_scale(1, 1, 1), m_cachedTransform(1.0f) {}

        //! Destructor (default).
        ~TransformComponent() = default;

        //! Recompute cached transform.
        /*!
          Updates the cached transform to reflect the stored translation, rotation, and scale data.
          If the cached transform has any changes (e.g. via SetTransform()) that are not reflected in the stored translation, rotation,
          and scale data, they will be lost.
          This function is automatically called from SetTranslation(), SetRotation(), and SetScale().
          \sa SetTranslation(), SetRotation, SetScale(), SetTransform()
        */
        void RecomputeTransform() {
            m_cachedTransform = glm::translate(glm::mat4(1.0f), m_translation) * glm::toMat4(m_rotation) * glm::scale(glm::mat4(1.0f), m_scale);
        }

        //! Get cached transform.
        /*!
          Returns the overall (cached) transform of this component.
          \return the cached transform
        */
        const glm::mat4& GetTransform() const {
            return m_cachedTransform;
        }

        //! Get translation.
        /*!
          Returns the translation data of this component.
          \return the translation data
        */
        const glm::vec3& GetTranslation() const {
            return m_translation;
        }
        
        //! Get rotation.
        /*!
          Returns the rotation data of this component.
          \return the rotation data
        */
        const glm::quat& GetRotation() const {
            return m_rotation;
        }
        
        //! Get scale.
        /*!
          Returns the scale data of this component.
          \return the scale data
        */
        const glm::vec3& GetScale() const {
            return m_scale;
        }

        //! Set translation.
        /*!
          Sets the translation data of this component to the specified translation vector.
          Automatically calls RecomputeTransform().
          \param newTranslation the new translation data
          \sa RecomputeTransform()
        */
        void SetTranslation(const glm::vec3& newTranslation) {
            m_translation = newTranslation;
            RecomputeTransform();
        }

        //! Set rotation.
        /*!
          Sets the rotation data of this component to the specified rotation quaternion.
          Automatically calls RecomputeTransform().
          \param newRotation the new rotation quaternion data
          \sa RecomputeTransform()
        */
        void SetRotation(const glm::quat& newRotation) {
            m_rotation = newRotation;
            RecomputeTransform();
        }

        //! Set rotation.
        /*!
          Sets the rotation data of this component to the specified rotation angle and axis.
          Automatically calls RecomputeTransform().
          \param angle the new rotation angle
          \param axis the new rotation axis
          \sa RecomputeTransform()
        */
        void SetRotation(const float angle, const glm::vec3& axis) {
            m_rotation = glm::angleAxis(angle, axis);
            RecomputeTransform();
        }

        //! Set scale.
        /*!
          Sets the scale data of this component to the specified scale vector.
          Automatically calls RecomputeTransform().
          \param newScale the new scale data
          \sa RecomputeTransform()
        */
        void SetScale(const glm::vec3& newScale) {
            m_scale = newScale;
            RecomputeTransform();
        }

        //! Set cached transform.
        /*!
          Sets the overall (cached) transform data of this component to the specified transformation matrix.
          Does NOT automatically call RecomputeTransform(). Changes made to the overall transformation via
          this function will stick until RecomputeTransform() is called.
          \param newTransform the new transformation data
          \sa RecomputeTransform()
        */
        void SetTransform(const glm::mat4& newTransform) {
            m_cachedTransform = newTransform;
        }
    };
}
