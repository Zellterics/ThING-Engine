#version 450
//LATER ADD CHECKS FOR DRAW INDEX, SO THE OUTLINE WONT BE OVER OTHER NON OUTLINED OBJECTS
layout(set = 0, binding = 0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(set = 0, binding = 1) uniform isampler2D idTex;      // rg (objectID, drawIndex)
layout(set = 0, binding = 2) uniform isampler2D  jfaResult; // rg = seed

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

bool isValidSeed(ivec2 seed, ivec2 fullSize)
{
    return seed.x >= 0 && seed.y >= 0 &&
           seed.x < fullSize.x && seed.y < fullSize.y;
}

ivec2 resolveSeed(
    ivec2 fullP,
    ivec2 halfP,
    ivec2 halfSize,
    ivec2 fullSize)
{
    ivec2 bestSeed = ivec2(-1);
    float bestDistance2 = 3.402823e38;

    for (int oy = -1; oy <= 1; ++oy) {
        for (int ox = -1; ox <= 1; ++ox) {
            ivec2 hp = halfP + ivec2(ox, oy);

            if (hp.x < 0 || hp.y < 0 ||
                hp.x >= halfSize.x || hp.y >= halfSize.y)
                continue;

            ivec2 candidate = texelFetch(jfaResult, hp, 0).xy;

            if (!isValidSeed(candidate, fullSize))
                continue;

            vec2 delta = vec2(candidate - fullP);
            float distance2 = dot(delta, delta);

            if (distance2 < bestDistance2) {
                bestDistance2 = distance2;
                bestSeed = candidate;
            }
        }
    }

    return bestSeed;
}

void main()
{
    float zoom = max((ubo.viewportSize.x * ubo.projection[0][0]) * 0.5, 1e-4);

    ivec2 fullSize = textureSize(idTex, 0);
    ivec2 halfSize = textureSize(jfaResult, 0);

    ivec2 fullP = clamp(
        ivec2(gl_FragCoord.xy),
        ivec2(0),
        fullSize - 1
    );

    ivec2 centerData = texelFetch(idTex, fullP, 0).rg;
    if (centerData.x >= 0)
        discard;

    ivec2 halfP = clamp(fullP / 2, ivec2(0), halfSize - 1);

    ivec2 seed = resolveSeed(fullP, halfP, halfSize, fullSize);

    if (!isValidSeed(seed, fullSize))
        discard;

    ivec2 seedData = texelFetch(idTex, seed, 0).rg;
    int id = seedData.x;

    if (id < 0)
        discard;

    OutlineData o = outlines[id];

    if (o.alive == 0)
        discard;

    float outer = max(o.outlineSize * zoom - 0.5, 0.0);

    vec2 seedPos = vec2(seed) + vec2(0.5);
    vec2 centerPos = vec2(fullP) + vec2(0.5);

    float centerDist = length(seedPos - centerPos);
    float aa = max(0.5, fwidth(centerDist) * 0.75);

    if (centerDist >= outer + aa + 0.36)
        discard;

    const vec2 sampleOffsets[4] = vec2[4](
        vec2(0.25, 0.25),
        vec2(0.75, 0.25),
        vec2(0.25, 0.75),
        vec2(0.75, 0.75)
    );

    float coverageSum = 0.0;

    for (int i = 0; i < 4; ++i) {
        vec2 samplePos = vec2(fullP) + sampleOffsets[i];
        float dist = length(seedPos - samplePos);

        coverageSum += 1.0 - smoothstep(
            outer - aa,
            outer + aa,
            dist
        );
    }

    float alpha = coverageSum * 0.25 * o.outlineColor.a;

    if (alpha <= 1e-4)
        discard;

    outColor = vec4(o.outlineColor.rgb, alpha);
}
