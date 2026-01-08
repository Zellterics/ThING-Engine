#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) flat in uint vObjectID;
layout(location = 2) in vec2 vLocalPos;
layout(location = 3) flat in uint vType; // 0 = Polygon, 1 = Circle

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outObjectID;
layout(location = 2) out vec4 outOutlineData;

void main() {
    if (vType == 0u) {
        outColor = vColor;
        outObjectID = vObjectID;
        outOutlineData = vec4(0.0);
        return;
    }
    
    float d = length(vLocalPos);
    float aa = fwidth(d);

    float alpha = 1.0 - smoothstep(1.0 - aa, 1.0 + aa, d);

    if (alpha <= 0.0)
        discard;

    outColor = vec4(vColor.rgb * alpha, alpha);
    outObjectID = vObjectID;
    outOutlineData = vec4(0.0);
}
