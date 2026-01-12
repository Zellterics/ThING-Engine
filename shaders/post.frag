#version 450

layout(set = 0, binding = 2) uniform sampler2D jfaResult;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    ivec2 size = textureSize(jfaResult, 0);

    // 🔑 Centro real del píxel
    vec2 pixel = fragUV * vec2(size) + vec2(0.5);

    vec2 seed = texture(jfaResult, fragUV).rg;

    if (seed == vec2(0.0))
        discard;

    float d = length(seed - pixel);

    float v = clamp(1.0 - d * 0.01, 0.0, 1.0);
    outColor = vec4(vec3(v), 1.0);
}
