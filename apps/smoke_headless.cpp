#include <cstdio>
#include <vector>
#include <glm/glm.hpp>
#include "render/frustum_cull.hpp"

int main(){
    voxelvk::ChunkBounds chunk{glm::vec3(-1.0f), glm::vec3(1.0f), 0u, 36u};
    std::vector<voxelvk::ChunkBounds> chunks{chunk};
    glm::mat4 vp(1.0f);
    std::vector<VkDrawIndirectCommand> draws;
    voxelvk::cpu_frustum_cull(chunks, vp, draws);
    std::printf("visible draws: %zu\n", draws.size());
    std::puts("smoke_headless: OK");
    return 0;
}
