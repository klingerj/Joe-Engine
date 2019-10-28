#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UBO_ViewProj_Inv {
    mat4 invProj;
    mat4 invView;
} ubo_viewProj_inv;

layout(binding = 1) uniform sampler2D gBufferAlbedo;
layout(binding = 2) uniform sampler2D gBufferNormal;
layout(binding = 3) uniform sampler2D gBufferDepth;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec2 fragPos;
layout(location = 0) out vec4 outColor;

void main() {
    // Read from G-buffers
    vec3 gBuffer_Albedo = texture(gBufferAlbedo, fragUV).xyz;
    vec3 gBuffer_Normal = texture(gBufferNormal, fragUV).xyz * 2 - 1;

    // Get world space position from fragment position and depth
    float depth = texture(gBufferDepth, fragUV).r;
    vec3 ndcPos = vec3(fragPos, depth);
    vec4 viewPos = ubo_viewProj_inv.invProj * vec4(ndcPos, 1.0);
    viewPos /= viewPos.w;
    vec3 worldPos = (ubo_viewProj_inv.invView * viewPos).xyz;
    
    float lambert = clamp(dot(normalize(vec3(20.0)), gBuffer_Normal), 0.0, 1.0);
    outColor = vec4(gBuffer_Albedo * lambert, 1.0);
}
