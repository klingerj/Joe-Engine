#version 450

layout (push_constant) uniform PushConstant {
    mat4 viewProj;
    uint instancedData[4];
} pushConstants;

layout(set = 0, binding = 0, std430) readonly buffer ssboModelMatrices {
  mat4 modelMatrices[];  
} ssboModelMatrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

void main() {
    mat4 modelMat = ssboModelMatrix.modelMatrices[gl_InstanceIndex + pushConstants.instancedData[0]];
    gl_Position = pushConstants.viewProj * modelMat * vec4(inPosition, 1.0);
}
