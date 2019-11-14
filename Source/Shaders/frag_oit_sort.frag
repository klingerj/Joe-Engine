#version 450
#extension GL_ARB_separate_shader_objects : enable

struct OITLinkedListNode {
  vec4 color; // RGB, Alpha
  vec4 depth; // Depth [0, 1], unused float3
};

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

void main() {
    ivec2 pixelIdx = ivec2(gl_FragCoord.xy - 0.5);
    ivec2 screenRes = ivec2(1280, 720);
    uint pixel = uint(pixelIdx.x + pixelIdx.y * screenRes.x); // TODO: pass screen width
    
    uint headPtr = ssboOITHP.headPointers[pixel];
    
    vec4 color = vec4(0);
    
    // TODO: actual sorting
    float depth = 1.0;
    if (headPtr != 0) {
        color = ssboOITLL.colorNodes[headPtr].color;
        depth = min(depth, ssboOITLL.colorNodes[headPtr].depth.x);
        headPtr = ssboOITNP.nextPointers[headPtr];
    }
    
    for (int i = 0; i < 1000; ++i) {
        if (headPtr != 0) {
            vec4 col = ssboOITLL.colorNodes[headPtr].color;
            depth = min(depth, ssboOITLL.colorNodes[headPtr].depth.x);
            
            color.rgb = col.rgb * col.a + color.rgb * color.a;
            color.a += col.a;
            if (color.a >= 1.0) {
                color.a = 1.0;
                break;
            }
            headPtr = ssboOITNP.nextPointers[headPtr];
        } else {
            break;
        }
    }
    
    gl_FragDepth = depth;
    outColor = color;
}
