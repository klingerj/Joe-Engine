#version 450
#extension GL_ARB_separate_shader_objects : enable

struct OITLinkedListNode {
  vec4 color; // RGB, Alpha
  vec4 depth; // Depth [0, 1], unused float3
};

#define OITLLNode struct OITLinkedListNode

layout(set = 0, binding = 0, std430) readonly buffer ssboOITLinkedList {
  OITLinkedListNode colorNodes[];
} ssboOITLL;

layout(set = 0, binding = 1, std430) readonly buffer ssboOITLinkedListNextPointers {
  uint nextPointers[];
} ssboOITNP;

layout(set = 0, binding = 2, std430) readonly buffer ssboOITLinkedListHeadPointers {
  uint headPointers[];
} ssboOITHP;

layout(location = 0) out vec4 outColor;

#define NUM_FRAGS_PER_PIXEL 32
#define UINT_MAX 4294967295

void main() {
    ivec2 pixelIdx = ivec2(gl_FragCoord.xy - 0.5);
    ivec2 screenRes = ivec2(1280, 720);
    uint pixel = uint(pixelIdx.x + pixelIdx.y * screenRes.x); // TODO: pass screen width
    
    uint headPtr = ssboOITHP.headPointers[pixel];
    
    OITLinkedListNode fragments[NUM_FRAGS_PER_PIXEL];
    
    // Fill the array with the linked list of pixels
    int numFrags = NUM_FRAGS_PER_PIXEL;
    for (int i = 0; i < NUM_FRAGS_PER_PIXEL; ++i) {
        if (headPtr != UINT_MAX) {
            OITLinkedListNode node = ssboOITLL.colorNodes[headPtr];
            fragments[i] = node;
            headPtr = ssboOITNP.nextPointers[headPtr];
        } else {
            numFrags = i;
            break;
        }
    }
    
    // Insertion sort the fragment array
    for (int j = 1; j < numFrags; ++j) {
        OITLinkedListNode keyNode = fragments[j];
        int i = j - 1;
        while (i >= 0 && fragments[i].depth.r < keyNode.depth.r) {
            fragments[i + 1] = fragments[i];
            --i;
        }
        fragments[i + 1] = keyNode;
    }
    
    // Perform linear blending
    vec4 color = vec4(0);
    
    for (int i = 0; i < numFrags; ++i) {
        if (i == numFrags - 1) {
            gl_FragDepth = fragments[i].depth.r;
        }
        vec4 currCol = fragments[i].color;
        color.rgb = currCol.rgb * currCol.a + color.rgb * (1.0 - currCol.a);
        //color.a = currCol.a + color.a * (1.0 - currCol.a);
        color.a = 1.0;
    }
    
    /*if (color.a >= 1.0) {
        color /= color.a;
    }*/
    outColor = color;
}
