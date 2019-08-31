#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO_ViewProj {
    mat4 viewProj;
} ubo_viewProj;

layout (binding = 1) uniform UBODynamic_ModelMat {
	mat4 model;
} uboDynamicModelMatInstance;

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
    vec3 pos = (uboDynamicModelMatInstance.model * vec4(inPosition, 1.0)).xyz;
    fragPos = pos;
    fragNor = normalize((inverse(transpose(uboDynamicModelMatInstance.model)) * vec4(inNormal, 0)).xyz);
    gl_Position = ubo_viewProj.viewProj * vec4(pos, 1.0);
    fragUV = inUV;
    fragColor = inColor;
}