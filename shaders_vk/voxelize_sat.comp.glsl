#version 450
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(push_constant) uniform TrianglePC {
    vec4 v0;
    vec4 v1;
    vec4 v2;
} pc;

// Placeholder compute shader for voxelization.
void main() {
    // Implementation would voxelize the triangle defined by pc.v0, pc.v1 and pc.v2.
}
