#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    vec3 texColor = texture(albedo, fragUV).xyz;
    outColor = vec4(fragColor * texColor, 1.0);
}
