#version 450

layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(set = 0, binding = 1) uniform isampler2D idTex;      // rg (objectID, drawIndex)
layout(set = 0, binding = 2) uniform sampler2D  jfaResult; // rg = seed

layout(location = 0) out vec4 outColor;

struct OutlineData {
    vec4  outlineColor;
    float outlineSize;
    uint  groupID;
    uint  alive;
    uint  _pad0;
};

layout(set = 0, binding = 3, std430) readonly buffer OutlineBuffer {
    OutlineData outlines[];
};

float ringSD(float sd, float inner, float outer, float aa)
{
    float aIn  = smoothstep(-inner - aa, -inner + aa, sd);
    float aOut = 1.0 - smoothstep(outer - aa, outer + aa, sd);
    return aIn * aOut;
}

bool isValidSeed(vec2 s, ivec2 size)
{
    if (s.x < 0.0 || s.y < 0.0) return false;
    if (s.x >= float(size.x) || s.y >= float(size.y)) return false;

    vec2 f = fract(s);
    return abs(f.x - 0.5) < 0.02 && abs(f.y - 0.5) < 0.02;
}

void main()
{
    float zoom = max((ubo.viewportSize.x * ubo.projection[0][0]) * 0.5, 1e-4);

    ivec2 texSize  = textureSize(jfaResult, 0);
    ivec2 p        = clamp(ivec2(gl_FragCoord.xy), ivec2(0), texSize - 1);
    vec2  worldPos = vec2(p) + vec2(0.5);

    ivec2 centerData = texelFetch(idTex, p, 0).rg;
    int centerID        = centerData.x;
    int centerDrawIndex = centerData.y;

    bool centerHasOutline = false;
    OutlineData centerO;

    if (centerID >= 0) {
        centerO = outlines[centerID];
        centerHasOutline = (centerO.alive != 0);
    }

    vec2 centerSeed = texelFetch(jfaResult, p, 0).xy;
    if (!isValidSeed(centerSeed, texSize))
        discard;

    vec2 s = centerSeed;

    ivec2 d = texelFetch(idTex, ivec2(s), 0).rg;
    int id        = d.x;
    int drawIndex = d.y;

    OutlineData o = outlines[id];

    if (o.alive == 0)
        discard;

    bool wins;

    if (!centerHasOutline) {
        wins =
            (drawIndex > centerDrawIndex) ||
            (drawIndex == centerDrawIndex && id > centerID);
    }
    else if (id == centerID) {
        wins =
            (drawIndex > centerDrawIndex) ||
            (drawIndex == centerDrawIndex && id >= centerID);
    }
    else {
        wins =
            (drawIndex > centerDrawIndex) ||
            (drawIndex == centerDrawIndex && id > centerID);
    }

    if (!wins)
        discard;

    float R_world  = 4.0;
    float aa_world = 1.5;

    float R             = R_world * zoom;
    float outlineOuter  = o.outlineSize * zoom;
    float aa            = aa_world * zoom;
    float outlineInner  = max(R - 0.01 * zoom, 0.0);

    float dist = length(s - worldPos);
    float sd   = dist - R;

    bool touches =
        (dist <= outlineOuter + aa) &&
        (sd   >= -outlineInner);

    if (!touches)
        discard;

    float a     = ringSD(sd, outlineInner, outlineOuter, aa);
    float alpha = clamp(a * 1.35, 0.0, 1.0) * o.outlineColor.a;

    if (alpha <= 1e-4)
        discard;

    outColor = vec4(o.outlineColor.rgb, alpha);
}
