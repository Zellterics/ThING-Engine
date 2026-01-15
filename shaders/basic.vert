#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 inUV;

layout(location = 2)  in vec2  iPosition;   // p0
layout(location = 3)  in vec2  iScale;      // p1 (for line)
layout(location = 4)  in float iRotation;   // thickness (for line)
layout(location = 5)  in float iOutlineSize;
layout(location = 6)  in uint  iObjectID;
layout(location = 7)  in uint  iGroupID;
layout(location = 8)  in vec4  iColor;
layout(location = 9)  in vec4  iOutlineColor;
layout(location = 10) in int   iDrawIndex;
layout(location = 11) in uint  iAlive;
layout(location = 12) in uint  iType;

layout(location = 0) out vec4 vColor;
layout(location = 1) flat out uint vObjectID;
layout(location = 2) out vec2 vLocalPos;
layout(location = 3) flat out uint vType;
layout(location = 4) flat out uint vOutlineSize;
layout(location = 5) flat out int  vOutDrawIndex;

const uint TYPE_LINE = 2u; // InstanceType::Line

void main() {
    if (iAlive == 0u) {
        gl_Position   = vec4(2.0, 2.0, 0.0, 1.0);
        vColor        = vec4(0.0);
        vObjectID     = 0u;
        vLocalPos     = vec2(0.0);
        vType         = 0u;
        vOutlineSize  = 0u;
        return;
    }

    vType         = iType;
    vColor        = iColor;
    vObjectID     = iObjectID;
    vOutlineSize  = uint(iOutlineSize);
    vOutDrawIndex = iDrawIndex;

    if (iType == TYPE_LINE) {

        vec2 p0 = iPosition;
        vec2 p1 = iScale;

        float t    = (inPos.x + 1.0) * 0.5; // [-1,1] → [0,1]
        float side = inPos.y;

        vec2 dir = p1 - p0;
        float len = length(dir);

        if (len < 1e-6) {
            gl_Position = vec4(2.0);
            return;
        }

        dir /= len;
        vec2 normal = vec2(-dir.y, dir.x);

        vec2 pos = mix(p0, p1, t);
        pos += normal * side * (iRotation * 0.5);

        gl_Position = ubo.projection * vec4(pos, 0.0, 1.0);
        vLocalPos   = vec2(t, side);
        return;
    }

    vLocalPos = inPos;

    vec2 local = inPos * iScale;

    float c = cos(iRotation);
    float s = sin(iRotation);

    vec2 rotated = vec2(
        c * local.x - s * local.y,
        s * local.x + c * local.y
    );

    vec2 worldPos = rotated + iPosition;
    gl_Position   = ubo.projection * vec4(worldPos, 0.0, 1.0);
}
