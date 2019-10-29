#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform UBO_ViewProj_Shadow {
    mat4 viewProj;
} ubo_viewProj_Shadow;

layout(set = 0, binding = 1) uniform sampler2D albedoMap;
layout(set = 0, binding = 2) uniform sampler2D roughnessMap;
layout(set = 0, binding = 3) uniform sampler2D metallicMap;
layout(set = 0, binding = 4) uniform sampler2D normalMap;
layout(set = 0, binding = 5) uniform sampler2D shadowMap;

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
    
    vec3 worldPos = fragPos + normalize(vec3(20.0) - fragPos) * 0.01;
    vec4 pointShadow = ubo_viewProj_Shadow.viewProj * vec4(worldPos, 1.0);
    pointShadow.xy = pointShadow.xy * 0.5 + 0.5;
    
    float shadowColor = texture(shadowMap, pointShadow.xy).r;
    float shadow = step(pointShadow.z, shadowColor);
    shadow = max(shadow, 0.15);
    float lambert = clamp(dot(normalize(vec3(20.0)), normal_tan), 0.0, 1.0);
    
    outColor = vec4(fragColor * albedo * lambert * shadow, 0.2);
}
