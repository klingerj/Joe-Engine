#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D albedoMap;
layout(binding = 1) uniform sampler2D roughnessMap;
layout(binding = 2) uniform sampler2D metallicMap;
layout(binding = 3) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

layout(location = 0) out vec4 gbuffers[2];

void main() {
    vec3 albedo = texture(albedoMap, fragUV).xyz;
    float roughness = texture(roughnessMap, fragUV).r;
    float metallic = texture(metallicMap, fragUV).r;
    vec3 normal_tan = texture(normalMap, fragUV).xyz;
    
    gbuffers[0] = vec4(fragColor * albedo, roughness);
    
    // TODO: actual normal mapping
    vec3 fakeNormal = 0.5 * (normal_tan + fragNor);
    gbuffers[1] = vec4(fakeNormal * 0.5 + 0.5, metallic);
}
