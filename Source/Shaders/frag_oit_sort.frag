#version 450
#extension GL_ARB_separate_shader_objects : enable

struct OITLinkedListNode {
  vec4 colorDepth; // RGB, Depth [0, 1]
};

layout(set = 0, binding = 0, std430) buffer ssboOITLinkedList {
  OITLinkedListNode colorNodes[];
} ssboOITLL;

layout(set = 0, binding = 1, std430) buffer ssboOITLinkedListNextPointers {
  uint nextPointers[];
} ssboOITNP;

layout(set = 0, binding = 2, std430) buffer ssboOITLinkedListHeadPointers {
  uint headPointers[];
} ssboOITHP;

layout(location = 0) out vec4 outColor;

void main() {
    // TODO: read from this pixel's linked list and blend the colors front-to-back
    vec4 color;
    
    ivec2 pixelIdx = ivec2(gl_FragCoord.xy - 0.5);
    ivec2 screenRes = ivec2(1280, 720);
    uint pixel = uint(pixelIdx.x + pixelIdx.y * screenRes.x); // TODO: pass screen width
    
    color = ssboOITLL.colorNodes[ssboOITHP.headPointers[pixel]];
    
    outColor = color;
}
