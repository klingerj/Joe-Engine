#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D textureMap;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(textureMap, vec2(gl_PointCoord.s, 1.0 - gl_PointCoord.t));
    
    vec2 coord = gl_PointCoord.st * 2.0 - 1.0;
    color.a = clamp(0.3 - length(coord), 0.0, 1.0);
    
    outColor = color;
}
