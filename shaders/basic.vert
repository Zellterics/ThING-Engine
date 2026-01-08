#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 2) in vec2  iPosition;
layout(location = 3) in vec2  iScale;
layout(location = 4) in float iRotation;
layout(location = 5) in float iOutlineSize;
layout(location = 6) in uint  iObjectID;
layout(location = 7) in uint  iType;
layout(location = 8) in vec4  iColor;
layout(location = 9) in vec4  iOutlineColor;
layout(location = 10) in uint iAlive;

layout(location = 0) out vec4 vColor;
layout(location = 1) flat out uint vObjectID;
layout(location = 2) out vec2 vLocalPos;
layout(location = 3) flat out uint vType;

void main() {

    if (iAlive == 0u) {
        gl_Position = vec4(0.0);
        return;
    }

    vLocalPos = inPos;
    vType     = iType;

    vec2 local = inPos * iScale;

    float c = cos(iRotation);
    float s = sin(iRotation);

    vec2 rotated = vec2(
        c * local.x - s * local.y,
        s * local.x + c * local.y
    );

    vec2 worldPos = rotated + iPosition;

    gl_Position = ubo.projection * vec4(worldPos, 0.0, 1.0);

    vColor    = iColor;
    vObjectID = iObjectID;
}
