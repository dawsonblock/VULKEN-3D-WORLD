#version 460
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=2) in vec2 aUV;

layout(location=0) out vec3 vWorldPos;
layout(location=1) out vec3 vNormal;
layout(location=2) out vec2 vUV;

layout(std140, binding=0) uniform Camera {
    mat4 uModel;
    mat4 uView;
    mat4 uProj;
} camera;

void main(){
    vec4 world = camera.uModel * vec4(aPos,1.0);
    vWorldPos = world.xyz;
    vNormal = mat3(camera.uModel) * aNormal;
    vUV = aUV;
    gl_Position = camera.uProj * camera.uView * world;
}
