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

struct OITLinkedListNode {
  vec4 colorDepth; // RGB, Depth [0, 1]
};

layout(set = 2, binding = 0, std430) buffer ssboOITLinkedList {
  OITLinkedListNode colorNodes[];
} ssboOITLL;

layout(set = 2, binding = 1, std430) buffer ssboOITLinkedListNextPointers {
  uint nextPointers[];
} ssboOITNP;

layout(set = 2, binding = 2, std430) buffer ssboOITLinkedListHeadPointers {
  uint headPointers[];
} ssboOITHP;

layout(set = 2, binding = 3, std430) buffer ssboAtomicCounter {
    uint counter[4]; // array solely for alignment purposes. Only counter[0] is used.
} ssboAtomicCtr;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

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
    
    vec4 color = vec4(fragColor * albedo * lambert * shadow * 0.2, gl_FragCoord.z); // TODO: make sure this is right
    uint headPtr = atomicAdd(ssboAtomicCtr.counter[0], 1);
    // TODO: make sure we don't have to worry about which corner this is relative to. Don't think so tho.
    ivec2 pixelIdx = ivec2(gl_FragCoord.xy - vec2(0.5));
    //uint pixel = uint(pixelIdx.x + pixelIdx.y * screenRes.x); // TODO: pass screen width
    uint pixel = 0;
    
    // Create and write the color-depth node for the linked list
    OITLinkedListNode oitNode;
    oitNode.colorDepth = color;
    // Set the color for the new atomic counter value to be this fragment
    ssboOITLL.colorNodes[headPtr] = oitNode;
    
    // Update the head pointer for this pixel
    uint oldHeadPtr = atomicExchange(ssboOITHP.headPointers[headPtr], headPtr);
    ssboOITHP.headPointers[pixel] = headPtr;
    
    // Update the next pointers for the head pointer with the old head pointer data
    ssboOITNP.nextPointers[headPtr] = oldHeadPtr;
}
