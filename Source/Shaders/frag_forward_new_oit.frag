#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D albedoMap;
layout(set = 0, binding = 1) uniform sampler2D roughnessMap;
layout(set = 0, binding = 2) uniform sampler2D metallicMap;
layout(set = 0, binding = 3) uniform sampler2D normalMap;

struct OITLinkedListNode {
  vec4 color; // RGB, Alpha
  vec4 depth; // Depth [0, 1], unused float3
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
    uint counter[4];
} ssboAtomicCtr;

layout(early_fragment_tests) in;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 2) in vec3 fragPos;
layout(location = 3) in vec3 fragNor;

// No out color! Only writes to SSBOs

void main() {
    vec3 albedo = texture(albedoMap, fragUV).xyz;
    float roughness = texture(roughnessMap, fragUV).r;
    float metallic = texture(metallicMap, fragUV).r;
    vec3 normal_tan = texture(normalMap, fragUV).xyz;
    
    vec4 color = vec4(fragColor * albedo, 0.5);
    uint headPtr = atomicAdd(ssboAtomicCtr.counter[0], 1);
    
    if (headPtr < ssboAtomicCtr.counter[1]) {
        ivec2 pixelIdx = ivec2(gl_FragCoord.xy - 0.5);
        ivec2 screenRes = ivec2(1280, 720);
        uint pixel = uint(pixelIdx.x + pixelIdx.y * screenRes.x); // TODO: pass screen width
        
        // Create and write the color-depth node for the linked list
        OITLinkedListNode oitNode;
        oitNode.color = color;
        oitNode.depth = vec4(gl_FragCoord.z, 0, 0, 0);
        // Set the color for the new atomic counter value to be this fragment
        ssboOITLL.colorNodes[headPtr] = oitNode;
        
        // Update the head pointer for this pixel
        uint oldHeadPtr = atomicExchange(ssboOITHP.headPointers[pixel], headPtr);
        
        // Update the next pointers for the head pointer with the old head pointer data
        ssboOITNP.nextPointers[headPtr] = oldHeadPtr;
    }
}
