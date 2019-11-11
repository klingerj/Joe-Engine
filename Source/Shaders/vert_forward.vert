#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform PushConstant {
    mat4 viewProj;
    uint instancedData[4];
} pushConstants;

layout(set = 1, binding = 0, std430) readonly buffer ssboModelMatrices {
  mat4 modelMatrices[];
} ssboModelMatrix;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;
layout(location = 2) out vec3 fragPos;
layout(location = 3) out vec3 fragNor;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 modelMat = ssboModelMatrix.modelMatrices[gl_InstanceIndex + pushConstants.instancedData[0]];
    vec3 pos = (modelMat * vec4(inPosition, 1.0)).xyz;
    fragPos = pos;
    fragNor = normalize((transpose(inverse(modelMat)) * vec4(inNormal, 0)).xyz);
    gl_Position = pushConstants.viewProj * vec4(pos, 1.0);
    fragUV = inUV;
    fragColor = inColor;
}
