#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D albedoMap;
layout(set = 0, binding = 1) uniform sampler2D roughnessMap;
layout(set = 0, binding = 2) uniform sampler2D metallicMap;
layout(set = 0, binding = 3) uniform sampler2D normalMap;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedo = texture(albedoMap, fragUV).xyz;
    float roughness = texture(roughnessMap, fragUV).r;
    float metallic = texture(metallicMap, fragUV).r;
    vec3 normal_tan = texture(normalMap, fragUV).xyz;
    
    float lambert = clamp(dot(normalize(vec3(20.0)), normal_tan), 0.0, 1.0);
    outColor = vec4(fragColor * albedo * lambert, 0.2);
}
