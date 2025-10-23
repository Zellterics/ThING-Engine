#version 450

layout(std140, set=0, binding=0) uniform UBO {
    mat4 projection;
    vec2 viewportSize;
} ubo;

layout(push_constant) uniform Transform {
    vec2  position; 
    float rotation; 
    float outlineSize; 
    float drawOutline;
    float windingSign;
    vec2  scale;        
    vec4  outlineColor;
} pc;

layout(location=0) in vec2 inPosition;
layout(location=1) in vec2 inNormal;
layout(location=2) in vec2 inNext;
layout(location=3) in vec2 inPrev;
layout(location=4) in vec3 inColor;

layout(location=0) out vec3 fragColor;

vec2 rot2(vec2 v, float c, float s){ return vec2(c*v.x - s*v.y, s*v.x + c*v.y); }

float projectionParity(){
    mat2 A = mat2( ubo.projection[0][0], ubo.projection[1][0],
                   ubo.projection[0][1], ubo.projection[1][1] );
    float detA = determinant(A);
    return detA >= 0.0 ? 1.0 : -1.0;
}

void main(){
    const float EPS = 1e-4;

    float c = cos(pc.rotation), s = sin(pc.rotation);

    vec2 P     = rot2(inPosition * pc.scale, c, s) + pc.position;
    vec2 Pprev = rot2(inPrev     * pc.scale, c, s) + pc.position;
    vec2 Pnext = rot2(inNext     * pc.scale, c, s) + pc.position;

    vec4 clipP   = ubo.projection * vec4(P,     0.0, 1.0);
    vec4 clipPrv = ubo.projection * vec4(Pprev, 0.0, 1.0);
    vec4 clipNxt = ubo.projection * vec4(Pnext, 0.0, 1.0);

    vec2 ndcP   = clipP.xy   / max(clipP.w,   EPS);
    vec2 ndcPrv = clipPrv.xy / max(clipPrv.w, EPS);
    vec2 ndcNxt = clipNxt.xy / max(clipNxt.w, EPS);

    vec2 vpAbs = abs(ubo.viewportSize);
    vec2 N2P   = 0.5 * vpAbs;
    vec2 P2N   = vec2(2.0/vpAbs.x, 2.0/vpAbs.y);
    float signY = (ubo.viewportSize.y >= 0.0) ? 1.0 : -1.0;

    vec2 sp    = ndcP   * N2P;
    vec2 spPrv = ndcPrv * N2P;
    vec2 spNxt = ndcNxt * N2P;

    vec2 t0 = sp - spPrv; float l0 = length(t0); t0 = (l0 > EPS) ? (t0/l0) : vec2(1,0);
    vec2 t1 = spNxt - sp; float l1 = length(t1); t1 = (l1 > EPS) ? (t1/l1) : vec2(1,0);

    float crossZ = t0.x*t1.y - t0.y*t1.x;

    float screenParity = projectionParity() * signY;

    float outward = pc.windingSign * screenParity;
    vec2 en0 = outward * vec2(t0.y, -t0.x);
    vec2 en1 = outward * vec2(t1.y, -t1.x);

    vec2 sum = en0 + en1;
    float sumLen = length(sum);
    vec2 miterDirPx = (sumLen > EPS) ? (sum / sumLen) : en0;

    float denom = max(dot(normalize(miterDirPx), normalize(en0)), 0.2);
    float miterScale = min(1.0/denom, 4.0);

    bool isConcave = (crossZ * outward) < 0.0;
    if (isConcave) { miterDirPx = en0; miterScale = 1.0; }

    vec4 clip = clipP;
    if (pc.drawOutline > 0.5) {
        vec2 offPx  = normalize(miterDirPx) * (pc.outlineSize * miterScale);
        vec2 offNDC = vec2(offPx.x * P2N.x, offPx.y * P2N.y * signY);
        clip.xy += offNDC * clip.w;
        fragColor = pc.outlineColor.rgb;
    } else {
        fragColor = inColor;
    }

    gl_Position = clip;
}
