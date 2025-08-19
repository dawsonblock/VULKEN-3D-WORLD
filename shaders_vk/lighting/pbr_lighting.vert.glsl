#version 460
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;
layout(location=0) out vec3 vWorldPos;
layout(location=1) out vec3 vNormal;
layout(location=2) out vec2 vUV;
layout(push_constant) uniform PC {
    mat4 model;
    mat4 viewProj;
    vec3 camPos;
    float _pad0;
    vec3 lightDir;
    float _pad1;
} pc;
void main(){
    vec4 wPos = pc.model * vec4(inPos,1.0);
    vWorldPos = wPos.xyz;
    vNormal = mat3(pc.model) * inNormal;
    vUV = inUV;
    gl_Position = pc.viewProj * wPos;
}
