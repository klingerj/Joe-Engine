#version 450

layout (push_constant) uniform PushConstant {
    mat4 viewProj;
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

void main() {
    gl_Position = pushConstants.viewProj * pushConstants.model * vec4(inPosition, 1.0);
}
