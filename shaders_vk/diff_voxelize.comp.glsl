#version 450
layout(local_size_x = 64) in;

layout(set = 0, binding = 0) readonly buffer Points {
    vec4 positions[];
};

layout(set = 0, binding = 1, r32f) uniform coherent image3D uOccupancy;
layout(set = 0, binding = 2, r32f) uniform coherent image3D uGradX;
layout(set = 0, binding = 3, r32f) uniform coherent image3D uGradY;
layout(set = 0, binding = 4, r32f) uniform coherent image3D uGradZ;

layout(push_constant) uniform Params {
    uint point_count;
} pc;

void main() {
    uint idx = gl_GlobalInvocationID.x;
    if (idx >= pc.point_count) return;

    vec3 p = positions[idx].xyz;
    ivec3 size = imageSize(uOccupancy);
    if (any(lessThan(p, vec3(0.0))) || any(greaterThanEqual(p, vec3(size)))) return;


    ivec3 base = ivec3(floor(p));
    if (any(lessThan(base, ivec3(0))) || any(greaterThan(base, size - ivec3(1)))) return;
    vec3 frac = p - vec3(base);

    for (int dx = 0; dx < 2; ++dx) {
        for (int dy = 0; dy < 2; ++dy) {
            for (int dz = 0; dz < 2; ++dz) {
                ivec3 vox = base + ivec3(dx, dy, dz);
                if (any(lessThan(vox, ivec3(0))) || any(greaterThanEqual(vox, size))) continue;
                float wx = dx == 1 ? frac.x : 1.0 - frac.x;
                float wy = dy == 1 ? frac.y : 1.0 - frac.y;
                float wz = dz == 1 ? frac.z : 1.0 - frac.z;
                float w = wx * wy * wz;
                imageAtomicAdd(uOccupancy, vox, w);
                float gx = (dx == 1 ? 1.0 : -1.0) * wy * wz;
                float gy = wx * (dy == 1 ? 1.0 : -1.0) * wz;
                float gz = wx * wy * (dz == 1 ? 1.0 : -1.0);
                imageAtomicAdd(uGradX, vox, gx);
                imageAtomicAdd(uGradY, vox, gy);
                imageAtomicAdd(uGradZ, vox, gz);
            }
        }
                sOccupancy[gl_LocalInvocationIndex] += w;
                float gx = (dx == 1 ? 1.0 : -1.0) * wy * wz;
                float gy = wx * (dy == 1 ? 1.0 : -1.0) * wz;
                float gz = wx * wy * (dz == 1 ? 1.0 : -1.0);
                sGradX[gl_LocalInvocationIndex] += gx;
                sGradY[gl_LocalInvocationIndex] += gy;
                sGradZ[gl_LocalInvocationIndex] += gz;
            }
        }
    }

    // Synchronize threads in the workgroup
    barrier();

    // Each thread writes its accumulated value to the global images
    ivec3 vox = ivec3(floor(p));
    if (all(greaterThanEqual(vox, ivec3(0))) && all(lessThan(vox, size))) {
        imageAtomicAdd(uOccupancy, vox, sOccupancy[gl_LocalInvocationIndex]);
        imageAtomicAdd(uGradX, vox, sGradX[gl_LocalInvocationIndex]);
        imageAtomicAdd(uGradY, vox, sGradY[gl_LocalInvocationIndex]);
        imageAtomicAdd(uGradZ, vox, sGradZ[gl_LocalInvocationIndex]);
    }
}
