#version 450
layout(local_size_x = 4, local_size_y = 4, local_size_z = 4) in;

layout(set = 0, binding = 0, r8ui) uniform readonly uimage3D uVoxels;

struct Triangle { vec4 p0; vec4 p1; vec4 p2; };
layout(set = 0, binding = 1, std430) buffer Triangles { Triangle tris[]; };
layout(set = 0, binding = 2, std430) buffer Counter { uint count; };

layout(push_constant) uniform Params {
    ivec3 chunkSize;
    float voxelSize;
    vec3 origin;
} params;

void emitQuad(vec3 a, vec3 b, vec3 c, vec3 d) {
    uint idx = atomicAdd(count, 2);
    tris[idx].p0 = vec4(a, 1.0);
    tris[idx].p1 = vec4(b, 1.0);
    tris[idx].p2 = vec4(c, 1.0);
    tris[idx + 1].p0 = vec4(a, 1.0);
    tris[idx + 1].p1 = vec4(c, 1.0);
    tris[idx + 1].p2 = vec4(d, 1.0);
}

void main() {
    ivec3 pos = ivec3(gl_GlobalInvocationID);
    if (any(greaterThanEqual(pos, params.chunkSize))) return;
    uint vox = imageLoad(uVoxels, pos).r;
    if (vox == 0u) return;
    vec3 base = params.origin + vec3(pos) * params.voxelSize;
    vec3 d = vec3(params.voxelSize);
    if (pos.x == 0 || imageLoad(uVoxels, pos - ivec3(1, 0, 0)).r == 0u)
        emitQuad(base, base + vec3(0, d.y, 0), base + vec3(0, d.y, d.z), base + vec3(0, 0, d.z));
    if (pos.x == params.chunkSize.x - 1 || imageLoad(uVoxels, pos + ivec3(1, 0, 0)).r == 0u)
        emitQuad(base + vec3(d.x, 0, 0), base + vec3(d.x, 0, d.z), base + vec3(d.x, d.y, d.z), base + vec3(d.x, d.y, 0));
    if (pos.y == 0 || imageLoad(uVoxels, pos - ivec3(0, 1, 0)).r == 0u)
        emitQuad(base, base + vec3(d.x, 0, 0), base + vec3(d.x, 0, d.z), base + vec3(0, 0, d.z));
    if (pos.y == params.chunkSize.y - 1 || imageLoad(uVoxels, pos + ivec3(0, 1, 0)).r == 0u)
        emitQuad(base + vec3(0, d.y, 0), base + vec3(0, d.y, d.z), base + vec3(d.x, d.y, d.z), base + vec3(d.x, d.y, 0));
    if (pos.z == 0 || imageLoad(uVoxels, pos - ivec3(0, 0, 1)).r == 0u)
        emitQuad(base, base + vec3(0, d.y, 0), base + vec3(d.x, d.y, 0), base + vec3(d.x, 0, 0));
    if (pos.z == params.chunkSize.z - 1 || imageLoad(uVoxels, pos + ivec3(0, 0, 1)).r == 0u)
        emitQuad(base + vec3(0, 0, d.z), base + vec3(d.x, 0, d.z), base + vec3(d.x, d.y, d.z), base + vec3(0, d.y, d.z));
}
