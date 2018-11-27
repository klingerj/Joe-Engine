#pragma once

#include "../utils/Common.h"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/norm.hpp"

class Camera {
private:
    glm::vec3 eye;
    glm::vec3 ref;
    glm::vec3 look;
    glm::vec3 right;
    glm::vec3 up;
    float aspect;
    float nearPlane, farPlane;

public:
    Camera() : Camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), 0.0f, SCENE_VIEW_NEAR_PLANE, SCENE_VIEW_FAR_PLANE) {}
    Camera(glm::vec3 e, glm::vec3 r, float a, float n, float f) : eye(e), ref(r), aspect(a), nearPlane(n), farPlane(f) { ComputeAttributes();  }
    ~Camera() {}

    void ComputeAttributes() {
        look = glm::normalize(ref - eye);
        right = glm::normalize(glm::cross(look, WORLD_UP));
        up = glm::normalize(glm::cross(right, look));
    }

    // Camera Movement
    void TranslateAlongLook(float amount) {
        ref += look * amount;
        eye += look * amount;
    }
    void TranslateAlongRight(float amount) {
        ref += right * amount;
        eye += right * amount;
    }
    void TranslateAlongUp(float amount) {
        ref += up * amount;
        eye += up * amount;
    }

    void SetAspect(float a) {
        aspect = a;
    }
    void SetEye(glm::vec3 e) {
        eye = e;
        ComputeAttributes();
    }

    // Getters
    glm::mat4 GetView() const {
        glm::mat4 t = glm::mat4(1.0);
        t[3] = glm::vec4(-eye.x, -eye.y, -eye.z, 1.0);

        glm::mat4 o = glm::mat4(1.0);
        o[0] = glm::vec4(right.x, up.x, look.x, 0.0);
        o[1] = glm::vec4(right.y, up.y, look.y, 0.0);
        o[2] = glm::vec4(right.z, up.z, look.z, 0.0);

        return o * t;
    }
    glm::mat4 GetProj() const {
        return glm::perspective(FOVY, aspect, nearPlane, farPlane);
    }
};
