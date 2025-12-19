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
    uint  objectID;
} pc;

layout(location=0) in vec2 inPosition;
layout(location=1) in vec2 inNormal;
layout(location=2) in vec2 inNext;
layout(location=3) in vec2 inPrev;
layout(location=4) in vec3 inColor;

layout(location=0) out vec4 vColor;
layout(location=1) out flat uint vObjectID;
layout(location=2) out vec4 vOutlineColor;

void main(){
    float c = cos(pc.rotation), s = sin(pc.rotation);
    vec2 rotated = vec2(c * inPosition.x - s * inPosition.y,
                        s * inPosition.x + c * inPosition.y);
    vec2 worldPos = rotated * pc.scale + pc.position;
    gl_Position = ubo.projection * vec4(worldPos, 0.0, 1.0);

    vColor = vec4(inColor, 1.0);
    vObjectID = pc.objectID;
}
