#version 450
layout(location = 0) in vec2  fragLocal;
layout(location = 1) in vec3  fragFillColor;
layout(location = 2) in float fragInnerRadiusN;
layout(location = 3) in vec4  fragOutlineColor;
layout(location = 4) in float fragOuterRadiusN;

layout(location = 0) out vec4 outColor;

void main() {
    float d  = length(fragLocal);
    float aa = fwidth(d);

    float rIn  = clamp(fragInnerRadiusN, 0.0, 1.0);
    float rOut = clamp(fragOuterRadiusN, 0.0, 1.0);

    float insideOuter = 1.0 - smoothstep(rOut - aa, rOut + aa, d);
    float insideInner = 1.0 - smoothstep(rIn  - aa, rIn  + aa, d);
    float outlineMask = max(insideOuter - insideInner, 0.0);

    float alphaFill    = insideInner;
    float alphaOutline = outlineMask * fragOutlineColor.a;
    float a            = alphaFill + alphaOutline;

    vec3 rgbStraight = (a > 1e-6)
        ? ( (fragFillColor * alphaFill + fragOutlineColor.rgb * alphaOutline) / a )
        : vec3(0.0);

    if (a <= 0.0) discard;
    outColor = vec4(rgbStraight, a);
}
