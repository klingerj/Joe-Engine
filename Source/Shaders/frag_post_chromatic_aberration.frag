#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D albedo;

layout(location = 0) in vec2 fragUV;
layout(location = 1) in vec2 fragPos;
layout(location = 0) out vec4 outColor;

void main() {
    float r = texture(albedo, fragUV + vec2(0.01, 0.0)).r;
    float g = texture(albedo, fragUV + vec2(0.015, 0.01)).g;
    float b = texture(albedo, fragUV + vec2(-0.02, 0.01)).b;
    outColor = vec4(vec3(r, g, b), 1.0);
}
