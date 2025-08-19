#version 460
#extension GL_EXT_conservative_rasterization : enable

layout(location=0) in vec3 inPosition;

layout(push_constant) uniform Push {
    mat4 uMVP;
    int  uSlice;
} pc;

void main(){
    gl_Position = pc.uMVP * vec4(inPosition, 1.0);
}
