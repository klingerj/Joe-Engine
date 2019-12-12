#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D albedo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedoColor = texture(albedo, fragUV).xyz;
    // TODO: shadows and lambert's law here
    outColor = vec4(albedoColor * fragColor, 1.0);
}
