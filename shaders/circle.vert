#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(location = 0) in vec2 inQuadPos;
layout(location = 1) in vec2 inUV;           // no usado, pero lo dejo por si tu layout ya existe

layout(location = 2) in vec2  inCirclePos;
layout(location = 3) in float inCircleRadius;   // world units
layout(location = 4) in vec3  inCircleColor;
layout(location = 5) in float inOutlineSize;    // world units
layout(location = 6) in vec4  inOutlineColor;   // rgb=color, a=opacity (opcional)
layout(location = 7) in uint  inObjectID;

layout(location = 0) out vec2  fragLocal;
layout(location = 1) out vec3  fragFillColor;
layout(location = 2) out float fragInnerRadiusN;   // rFill / rGeom
layout(location = 3) out vec4  fragOutlineColor;   // rgb=color, a=opacity (no se usa en post, solo rgb)
layout(location = 4) out float fragOuterRadiusN;   // rOut  / rGeom
layout(location = 5) flat out uint fragObjectID;
layout(location = 6) out float fragOutlinePacked;  // thickness packed [0..1]

void main() {
    vec2 local = inQuadPos;

    float rFill = max(inCircleRadius, 0.0);
    float outlineWorld = max(inOutlineSize, 0.0);
    float rOut  = rFill + outlineWorld;

    // Aproximar worldUnitsPerPixel para ortho (usa diag de la matriz)
    float sx = abs(ubo.projection[0][0]);
    float sy = abs(ubo.projection[1][1]);

    float worldPerPixelX = (sx > 0.0 && ubo.viewportSize.x > 0.0)
        ? (2.0 / (sx * ubo.viewportSize.x)) : 0.0;

    float worldPerPixelY = (sy > 0.0 && ubo.viewportSize.y > 0.0)
        ? (2.0 / (sy * ubo.viewportSize.y)) : 0.0;

    float worldPerPixel = max(worldPerPixelX, worldPerPixelY);

    // Padding AA alrededor del quad (en px -> world)
    const float AA_PAD_PX = 1.5;
    float padWorld = AA_PAD_PX * worldPerPixel;

    // Tamaño real del quad en world
    float rGeom = rOut + padWorld;

    // Empaquetar grosor en alpha en pixeles (clamp a MAX_OUTLINE_PX)
    const float MAX_OUTLINE_PX = 16.0;
    float outlinePx = (worldPerPixel > 0.0) ? (outlineWorld / worldPerPixel) : 0.0;
    outlinePx = clamp(outlinePx, 0.0, MAX_OUTLINE_PX);
    float packed = (MAX_OUTLINE_PX > 0.0) ? (outlinePx / MAX_OUTLINE_PX) : 0.0;

    vec2 worldPos = inCirclePos + local * rGeom;
    gl_Position = ubo.projection * vec4(worldPos, 0.0, 1.0);

    fragLocal         = local;
    fragFillColor     = inCircleColor;
    fragOutlineColor  = inOutlineColor;
    fragInnerRadiusN  = (rGeom > 0.0) ? (rFill / rGeom) : 0.0;
    fragOuterRadiusN  = (rGeom > 0.0) ? (rOut  / rGeom) : 0.0;
    fragObjectID      = inObjectID;
    fragOutlinePacked = packed;
}
