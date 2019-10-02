#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform PushConstant {
    mat4 viewProj;
    mat4 model;
} pushConstants;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = pushConstants.viewProj * pushConstants.model * vec4(inPosition, 1.0);
}
