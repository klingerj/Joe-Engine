#include "RotatorComponent.h"

using namespace JoeEngine;

void RotatorComponent::Update(JEEngineInstance* engineInstance, uint32_t id) {
    TransformComponent* trans = engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(id);
    trans->SetTransform(trans->GetTransform() * glm::rotate(glm::mat4(1.0f), 0.025f, glm::vec3(0, 0, 1)));
}
