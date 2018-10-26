#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 3) uniform sampler2D albedo;
layout(binding = 4) uniform sampler2D shadowMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedoColor = texture(albedo, fragUV).xyz;
    float shadowColor = texture(shadowMap, fragUV).r;
    outColor = vec4(vec3(shadowColor), 1.0);
}
