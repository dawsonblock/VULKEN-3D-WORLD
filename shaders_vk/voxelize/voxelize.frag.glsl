#version 460
#extension GL_EXT_conservative_rasterization : enable

layout(set=0, binding=0, r8ui) uniform uimage3D uOccupancy;

layout(push_constant) uniform Push {
    mat4 uMVP;
    int  uSlice;
} pc;

void main(){
    ivec3 coord = ivec3(int(gl_FragCoord.x), int(gl_FragCoord.y), pc.uSlice);
    imageAtomicOr(uOccupancy, coord, 1);
}
