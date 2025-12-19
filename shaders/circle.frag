#version 450

layout(location = 0) in vec2  fragLocal;
layout(location = 1) in vec3  fragFillColor;
layout(location = 2) in float fragInnerRadiusN;   // rFill / rGeom
layout(location = 3) in vec4  fragOutlineColor;   // rgb=color
layout(location = 4) in float fragOuterRadiusN;   // rOut  / rGeom
layout(location = 5) flat in uint fragObjectID;
layout(location = 6) in float fragOutlinePacked;  // [0..1] thicknessPx/MAX_OUTLINE_PX

layout(location = 0) out vec4 outColor;        // swapchain (premultiplied)
layout(location = 1) out uint outObjectID;     // ID buffer
layout(location = 2) out vec4 outOutlineData;  // rgb=color, a=packedThickness

void main() {
    float d  = length(fragLocal);
    float aa = fwidth(d);

    float rIn  = fragInnerRadiusN;
    float rOut = fragOuterRadiusN;

    // Mantener vivo hasta rOut para escribir attachments
    float covOut = 1.0 - smoothstep(rOut - aa, rOut + aa, d);
    if (covOut <= 0.0) discard;

    // Fill AA
    float covIn = 1.0 - smoothstep(rIn - aa, rIn + aa, d);

    // Umbral mínimo para que el borde AA no sea “tierra de nadie”
    const float EPS = 1.0 / 255.0;

    outColor = vec4(fragFillColor * covIn, covIn);

    // ID existe donde el fill existe (incluye borde AA)
    outObjectID = (covIn > EPS) ? fragObjectID : 0u;

    // Outline params guardados en el mismo dominio del ID
    outOutlineData = (covIn > EPS)
        ? vec4(fragOutlineColor.rgb, fragOutlinePacked)
        : vec4(0.0);
}
