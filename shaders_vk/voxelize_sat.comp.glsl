#version 450
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(set = 0, binding = 0, r32ui) uniform uimage3D uOccupancy;

layout(push_constant) uniform PC {
    vec3 v0;
    vec3 v1;
    vec3 v2;
    vec3 origin;
    float voxelSize;
    ivec3 minVoxel;
    ivec3 gridDim;
} pc;

bool planeBoxOverlap(vec3 normal, float d, vec3 halfSize) {
    vec3 vmin, vmax;
    for(int q = 0; q < 3; ++q) {
        if(normal[q] > 0.0) {
            vmin[q] = -halfSize[q];
            vmax[q] =  halfSize[q];
        } else {
            vmin[q] =  halfSize[q];
            vmax[q] = -halfSize[q];
        }
    }
    if(dot(normal, vmin) + d > 0.0) return false;
    if(dot(normal, vmax) + d >= 0.0) return true;
    return false;
}

bool triBoxOverlap(vec3 center, vec3 halfSize, vec3 v0, vec3 v1, vec3 v2) {
    vec3 tv0 = v0 - center;
    vec3 tv1 = v1 - center;
    vec3 tv2 = v2 - center;
    vec3 e0 = tv1 - tv0;
    vec3 e1 = tv2 - tv1;
    vec3 e2 = tv0 - tv2;

    vec3 axes[9] = vec3[9](
        vec3(0, -e0.z, e0.y),
        vec3(e0.z, 0, -e0.x),
        vec3(-e0.y, e0.x, 0),
        vec3(0, -e1.z, e1.y),
        vec3(e1.z, 0, -e1.x),
        vec3(-e1.y, e1.x, 0),
        vec3(0, -e2.z, e2.y),
        vec3(e2.z, 0, -e2.x),
        vec3(-e2.y, e2.x, 0)
    );

    for(int i = 0; i < 9; ++i) {
        vec3 axis = axes[i];
        if(length(axis) < 1e-6) continue;
        float r = halfSize.x * abs(axis.x) + halfSize.y * abs(axis.y) + halfSize.z * abs(axis.z);
        float p0 = dot(tv0, axis);
        float p1 = dot(tv1, axis);
        float p2 = dot(tv2, axis);
        float mn = min(p0, min(p1, p2));
        float mx = max(p0, max(p1, p2));
        if(mn > r || mx < -r) return false;
    }

    float minv, maxv;
    minv = min(tv0.x, min(tv1.x, tv2.x));
    maxv = max(tv0.x, max(tv1.x, tv2.x));
    if(minv > halfSize.x || maxv < -halfSize.x) return false;
    minv = min(tv0.y, min(tv1.y, tv2.y));
    maxv = max(tv0.y, max(tv1.y, tv2.y));
    if(minv > halfSize.y || maxv < -halfSize.y) return false;
    minv = min(tv0.z, min(tv1.z, tv2.z));
    maxv = max(tv0.z, max(tv1.z, tv2.z));
    if(minv > halfSize.z || maxv < -halfSize.z) return false;

    vec3 normal = cross(e0, e1);
    float d = -dot(normal, tv0);
    if(!planeBoxOverlap(normal, d, halfSize)) return false;
    return true;
}

void main() {
    ivec3 coord = ivec3(gl_GlobalInvocationID) + pc.minVoxel;
    if(any(greaterThanEqual(coord, pc.gridDim))) return;
    vec3 boxMin = pc.origin + vec3(coord) * pc.voxelSize;
    vec3 center = boxMin + vec3(pc.voxelSize * 0.5);
    vec3 halfSize = vec3(pc.voxelSize * 0.5);
    if(triBoxOverlap(center, halfSize, pc.v0, pc.v1, pc.v2)) {
        imageAtomicOr(uOccupancy, coord, 1u);
    }
}