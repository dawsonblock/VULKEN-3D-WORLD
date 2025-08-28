#version 450
layout(local_size_x = 64) in;

#define VERTICES_PER_DRAW 36

struct AABB { vec3 min; vec3 max; };
layout(set = 0, binding = 0, std430) readonly buffer Bounds { AABB bounds[]; };

struct DrawCmd {
    uint vertexCount;
    uint instanceCount;
    uint firstVertex;
    uint firstInstance;
};
layout(set = 0, binding = 1, std430) writeonly buffer Draws { DrawCmd draws[]; };
layout(set = 0, binding = 2, std430) buffer Counter { uint count; };

layout(push_constant) uniform Frustum {
    vec4 planes[6];
} fr;

bool aabbInFrustum(AABB b){
    for(int i=0;i<6;i++){
        vec4 p = fr.planes[i];
        vec3 positive = vec3(p.x>0?b.max.x:b.min.x,
                             p.y>0?b.max.y:b.min.y,
                             p.z>0?b.max.z:b.min.z);
        if(dot(p.xyz, positive) + p.w < 0.0) return false;
    }
    return true;
}

void main(){
    uint idx = gl_GlobalInvocationID.x;
    if(idx >= bounds.length()) return;
    if(!aabbInFrustum(bounds[idx])) return;
    uint outIdx = atomicAdd(count,1);
    draws[outIdx].vertexCount = VERTICES_PER_DRAW;
    draws[outIdx].instanceCount = 1;
    draws[outIdx].firstVertex = idx*VERTICES_PER_DRAW;
    draws[outIdx].firstInstance = 0;
}