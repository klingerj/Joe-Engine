#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D albedo;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec2 fragPos;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedoColor = texture(albedo, fragUV).xyz;
    outColor = vec4(albedoColor, 1.0);
}
