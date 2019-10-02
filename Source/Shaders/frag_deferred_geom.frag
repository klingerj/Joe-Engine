#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D albedo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

layout(location = 0) out vec4 gbuffers[2];

void main() {
    vec3 albedoColor = texture(albedo, fragUV).xyz;
    gbuffers[0] = vec4(albedoColor * fragColor, 1.0);
    gbuffers[1] = vec4(fragNor * 0.5 + 0.5, 1.0);
}
