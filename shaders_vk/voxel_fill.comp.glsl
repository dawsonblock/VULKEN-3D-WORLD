#version 450
layout (local_size_x = 8, local_size_y = 8, local_size_z = 1) in;

layout (set = 0, binding = 0, r8ui) uniform readonly  uimage3D uSurface;
layout (set = 0, binding = 1, r8ui) uniform writeonly uimage3D uSolid;

void main() {
    ivec2 xy = ivec2(gl_GlobalInvocationID.xy);
    ivec3 size = imageSize(uSurface);
    if (xy.x >= size.x || xy.y >= size.y) return;
    bool inside = false;
    for (int z = 0; z < size.z; ++z) {
        ivec3 pos = ivec3(xy, z);
        uint occ = imageLoad(uSurface, pos).r;
        if (occ != 0u) {
            inside = !inside;
            imageStore(uSolid, pos, uvec4(1, 0, 0, 0));
        } else {
            imageStore(uSolid, pos, uvec4(inside ? 1u : 0u, 0, 0, 0));
        }
    }
}
