#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(location = 0) in vec2 inQuadPos;
layout(location = 1) in vec2 inUV;

layout(location = 2) in vec2 inCirclePos;
layout(location = 3) in float inCircleRadius;
layout(location = 4) in vec3 inCircleColor;
layout(location = 5) in float inOutlineSize;
layout(location = 6) in vec4 inOutlineColor;

layout(location = 0) out vec2  fragLocal;
layout(location = 1) out vec3  fragFillColor;
layout(location = 2) out float fragInnerRadiusN;
layout(location = 3) out vec4  fragOutlineColor;
layout(location = 4) out float fragOuterRadiusN;

void main() {
    vec2 local = inQuadPos;

    float rFill = inCircleRadius;
    float rOut  = inCircleRadius + max(inOutlineSize, 0.0);

    float sx = abs(ubo.projection[0][0]);
    float sy = abs(ubo.projection[1][1]);

    float worldPerPixelX = (sx > 0.0 && ubo.viewportSize.x > 0.0)
        ? (2.0 / (sx * ubo.viewportSize.x)) : 0.0;
    float worldPerPixelY = (sy > 0.0 && ubo.viewportSize.y > 0.0)
        ? (2.0 / (sy * ubo.viewportSize.y)) : 0.0;

    float worldPerPixel  = max(worldPerPixelX, worldPerPixelY);
    const float AA_PAD_PX = 1.5;
    float padWorld = AA_PAD_PX * worldPerPixel;

    float rGeom = rOut + padWorld;

    vec2 worldPos = inCirclePos + local * rGeom;
    gl_Position = ubo.projection * vec4(worldPos, 0.0, 1.0);

    fragLocal         = local;
    fragFillColor     = inCircleColor;
    fragOutlineColor  = inOutlineColor;
    fragInnerRadiusN  = (rGeom > 0.0) ? (rFill / rGeom) : 0.0;
    fragOuterRadiusN  = (rGeom > 0.0) ? (rOut  / rGeom) : 0.0;
}
