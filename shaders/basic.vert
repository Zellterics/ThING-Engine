#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
} ubo;

layout(push_constant) uniform Transform {
    vec2  position;
    float rotation;
    float _pad0;
    vec2  scale;
    vec2  _pad1;
} pc;

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
    float c = cos(pc.rotation);
    float s = sin(pc.rotation);
    mat2 rot = mat2(c, -s, s, c);

    vec2 worldPos = rot * (inPosition * pc.scale) + pc.position;

    gl_Position = ubo.projection * vec4(worldPos, 0.0, 1.0);

    fragColor = inColor;
}
