#version 450
layout(location=0) in vec3 inPos;
layout(location=1) in vec3 inNormal;
layout(location=2) in vec2 inUV;

layout(location=0) out vec3 vNormal;
layout(location=1) out vec2 vUV;

layout(push_constant) uniform Push {
    mat4 model;
    mat4 viewProj;
    vec3 camPos;
} pc;

void main(){
    vNormal = mat3(pc.model) * inNormal;
    vUV = inUV;
    gl_Position = pc.viewProj * pc.model * vec4(inPos,1.0);
}
