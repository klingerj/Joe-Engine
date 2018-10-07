#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO_MVP {
    mat4 mvp;
} mvp;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragUV;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    gl_Position = mvp.mvp * vec4(inPosition, 1.0);
    fragUV = inUV;
    fragColor = inColor;
}
