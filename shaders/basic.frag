#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) flat in uint vObjectID;
layout(location = 2) in vec2 vLocalPos;
layout(location = 3) flat in uint vType;
layout(location = 4) flat in uint vOutlineSize;
layout(location = 5) flat in int  vDrawIndex;

layout(location = 0) out vec4  outColor;
layout(location = 1) out ivec2 outObjectID;
layout(location = 2) out vec4  outSeed;

const float MIN_DRAW_INDEX = -50000.0;
const float MAX_DRAW_INDEX =  50000.0;

const uint TYPE_CIRCLE = 1u;
const uint TYPE_LINE   = 2u;

void main()
{
    float alpha = vColor.a;

    // =====================================================
    // CIRCLE
    // =====================================================
    if (vType == TYPE_CIRCLE) {
        float r = 1.0;
        float d = length(vLocalPos);
        alpha *= 1.0 - smoothstep(r - 0.01, r + 0.01, d);
    }

    // =====================================================
    // LINE
    // =====================================================
    else if (vType == TYPE_LINE) {

        // vLocalPos.y ∈ [-1, +1]
        float dist = abs(vLocalPos.y);

        // line half-width normalized to 1.0
        float aa = fwidth(dist);
        alpha *= 1.0 - smoothstep(1.0 - aa, 1.0 + aa, dist);

        // OPTIONAL: clip ends cleanly (no caps yet)
        if (vLocalPos.x < 0.0 || vLocalPos.x > 1.0)
            discard;
    }

    if (alpha <= 0.0)
        discard;

    // =====================================================
    // DEPTH FROM DRAW INDEX
    // =====================================================
    float di = clamp(float(vDrawIndex), MIN_DRAW_INDEX, MAX_DRAW_INDEX);
    float depth01 = (di - MIN_DRAW_INDEX) / (MAX_DRAW_INDEX - MIN_DRAW_INDEX);
    gl_FragDepth = 1.0 - depth01;

    // =====================================================
    // OUTPUTS
    // =====================================================
    outColor = vec4(vColor.rgb, alpha);

    if (vOutlineSize > 0u) {
        outSeed     = vec4(gl_FragCoord.xy, 0.0, 0.0);
        outObjectID = ivec2(int(vObjectID), vDrawIndex);
    } else {
        outSeed     = vec4(-1.0, -1.0, 0.0, 0.0);
        outObjectID = ivec2(-1, vDrawIndex);
    }
}
