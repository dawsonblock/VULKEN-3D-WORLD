#version 450
#extension GL_EXT_samplerless_texture_functions : enable
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;
layout(set = 0, binding = 0, rgba16f) uniform readonly image3D uPrev;
layout(set = 0, binding = 1, rgba16f) uniform writeonly image3D uNext;
layout(push_constant) uniform PC {
    int step;
    int finalize;
} pc;
void main(){
    ivec3 gid = ivec3(gl_GlobalInvocationID);
    ivec3 dims = imageSize(uPrev);
    if (any(greaterThanEqual(gid, dims))) return;
    vec4 best = imageLoad(uPrev, gid);
    float bestDist = best.w > 0.5 ? length(best.xyz - vec3(gid)) : 1e30;
    for(int dz=-1; dz<=1; ++dz)
    for(int dy=-1; dy<=1; ++dy)
    for(int dx=-1; dx<=1; ++dx){
        ivec3 o = gid + ivec3(dx,dy,dz)*pc.step;
        if(any(lessThan(o, ivec3(0))) || any(greaterThanEqual(o, dims))) continue;
        vec4 c = imageLoad(uPrev, o);
        if(c.w <= 0.5) continue;
        float d = length(c.xyz - vec3(gid));
        if(d < bestDist){ best = c; bestDist = d; }
    }
    if(pc.finalize != 0) imageStore(uNext, gid, vec4(bestDist,0,0,0));
    else imageStore(uNext, gid, best);
}
