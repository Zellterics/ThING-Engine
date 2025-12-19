#version 450

layout(location = 0) in vec4 vColor;
layout(location = 1) in flat uint vObjectID;
layout(location = 2) in vec4 vOutlineColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out uint outObjectID;

void main() {
    outColor = vColor;
    outObjectID = vObjectID;
}