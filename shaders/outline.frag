#version 450

layout(set = 0, binding = 1) uniform usampler2D idTexture;
layout(set = 0, binding = 2) uniform sampler2D  outlineTexture;

layout(location = 0) in vec2 fragUV;
layout(location = 0) out vec4 outColor;

void main() {
    ivec2 texSize = textureSize(idTexture, 0);
    ivec2 uv = clamp(ivec2(fragUV * vec2(texSize)), ivec2(0), texSize - 1);

    uint centerID = texelFetch(idTexture, uv, 0).r;

    // -----------------------------
    // 1) Inward grow: 1 px hacia adentro
    // -----------------------------
    // (Solo aplica si estamos sobre un objeto)
    if (centerID != 0u) {
        bool nearBoundary = false;

        // Vecinos 4-dir (exacto 1 px cardinal)
        ivec2 offs[4] = ivec2[](
            ivec2( 1, 0),
            ivec2(-1, 0),
            ivec2( 0, 1),
            ivec2( 0,-1)
        );

        for (int i = 0; i < 4; ++i) {
            ivec2 n = uv + offs[i];
            if (any(lessThan(n, ivec2(0))) || any(greaterThanEqual(n, texSize)))
                continue;

            uint nid = texelFetch(idTexture, n, 0).r;
            if (nid != centerID) { // fondo u otro ID
                nearBoundary = true;
                break;
            }
        }

        if (nearBoundary) {
            // Pinta outline del MISMO objeto encima 1px hacia adentro
            vec4 o = texelFetch(outlineTexture, uv, 0);
            if (o.a > 0.0) {
                outColor = vec4(o.rgb, 1.0);
                return;
            }
        }
    }

    // -----------------------------
    // 2) Outline overlay: nunca se “desactiva” por IDs distintos
    // -----------------------------
    const float MAX_OUTLINE_PX = 16.0;
    int maxR = int(MAX_OUTLINE_PX);
    int maxR2 = maxR * maxR;

    bool hit = false;
    float bestD2 = 1e30;
    vec3 bestColor = vec3(0.0);
    float bestT = 0.0;

    // Busco outlines de OTROS IDs (para no pintar todo el objeto completo)
    for (int y = -maxR; y <= maxR; ++y) {
        for (int x = -maxR; x <= maxR; ++x) {
            int d2i = x*x + y*y;
            if (d2i > maxR2) continue;

            ivec2 n = uv + ivec2(x, y);
            if (any(lessThan(n, ivec2(0))) || any(greaterThanEqual(n, texSize)))
                continue;

            uint nid = texelFetch(idTexture, n, 0).r;
            if (nid == 0u) continue;
            if (nid == centerID) continue; // evita “pintar todo” del mismo ID

            vec4 o = texelFetch(outlineTexture, n, 0);
            float tPx = clamp(o.a * MAX_OUTLINE_PX, 0.0, MAX_OUTLINE_PX);
            if (tPx <= 0.0) continue;

            float d2 = float(d2i);
            float t2 = tPx * tPx;

            if (d2 <= t2) {
                // Quédate con el candidato más cercano (estable)
                if (d2 < bestD2) {
                    bestD2 = d2;
                    bestColor = o.rgb;
                    bestT = tPx;
                    hit = true;
                }
            }
        }
    }

    if (!hit)
        discard;

    // AA suave solo en el borde EXTERIOR del outline
    float dist = sqrt(bestD2);
    float alpha = 1.0 - smoothstep(bestT - 0.75, bestT + 0.75, dist);

    outColor = vec4(bestColor, alpha);
}
