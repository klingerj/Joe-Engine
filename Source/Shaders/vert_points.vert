#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (push_constant) uniform PushConstant {
    mat4 viewProj;
    uint instancedData[4];
} pushConstants;

layout(set = 0, location = 0) in vec3 inPosition;

out gl_PerVertex {
    vec4 gl_Position;
    float gl_PointSize;
};

void main() {
    gl_Position = pushConstants.viewProj * vec4(inPosition, 1.0);
    gl_PointSize = 6.0f;
}
