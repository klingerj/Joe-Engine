#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform UBO_ViewProj_Shadow {
    mat4 viewProj;
} ubo_viewProj_Shadow;

layout(binding = 3) uniform sampler2D albedo;
layout(binding = 4) uniform sampler2D shadowMap;
layout(binding = 5) uniform sampler2D gBufferColor;
layout(binding = 6) uniform sampler2D gBufferNormal;
layout(binding = 7) uniform sampler2D gBufferDepth;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec3 fragPos;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 albedoColor = texture(albedo, fragUV).xyz;
    vec3 gBufferColor = texture(gBufferColor, fragUV).xyz;
    /*vec4 pointShadow = ubo_viewProj_Shadow.viewProj * vec4(fragPos, 1.0);
    pointShadow.xyz += normalize(vec3(5.0) - pointShadow.xyz) * 0.005;
    pointShadow /= pointShadow.w;
    pointShadow.xy = pointShadow.xy * 0.5 + 0.5;
    float shadowColor = texture(shadowMap, pointShadow.xy).r;
    float shadow = step(pointShadow.z, shadowColor);*/
    outColor = vec4(gBufferColor, 1.0);
    //outColor = vec4(fragPos, 1.0);
}
