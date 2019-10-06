#include "RotatorComponent.h"

using namespace JoeEngine;

void RotatorComponent::Update(JEEngineInstance* engineInstance) {
    TransformComponent* trans = engineInstance->GetComponent<TransformComponent, JETransformComponentManager>(m_entityId);
    trans->SetTransform(trans->GetTransform() * glm::rotate(glm::mat4(1.0f), 0.025f, glm::vec3(0, 0, 1)));
}
