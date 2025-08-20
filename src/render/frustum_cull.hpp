#ifndef VOXELVK_FRUSTUM_CULL_HPP
#define VOXELVK_FRUSTUM_CULL_HPP

#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

namespace voxelvk {

struct ChunkBounds {
    glm::vec3 min;
    glm::vec3 max;
    uint32_t firstVertex;
    uint32_t vertexCount;
};

inline void cpu_frustum_cull(const std::vector<ChunkBounds>& chunks,
                             const glm::mat4& vp,
                             std::vector<VkDrawIndirectCommand>& out) {
    out.clear();
    glm::vec4 planes[6];
    auto m = vp;
    planes[0] = glm::vec4(m[0][3] + m[0][0], m[1][3] + m[1][0], m[2][3] + m[2][0], m[3][3] + m[3][0]);
    planes[1] = glm::vec4(m[0][3] - m[0][0], m[1][3] - m[1][0], m[2][3] - m[2][0], m[3][3] - m[3][0]);
    planes[2] = glm::vec4(m[0][3] + m[0][1], m[1][3] + m[1][1], m[2][3] + m[2][1], m[3][3] + m[3][1]);
    planes[3] = glm::vec4(m[0][3] - m[0][1], m[1][3] - m[1][1], m[2][3] - m[2][1], m[3][3] - m[3][1]);
    planes[4] = glm::vec4(m[0][3] + m[0][2], m[1][3] + m[1][2], m[2][3] + m[2][2], m[3][3] + m[3][2]);
    planes[5] = glm::vec4(m[0][3] - m[0][2], m[1][3] - m[1][2], m[2][3] - m[2][2], m[3][3] - m[3][2]);
    for (auto& p : planes) {
        float l = glm::length(glm::vec3(p));
        if (l > 0.0f) p /= l;
    }
    for (const auto& c : chunks) {
        bool outside = false;
        for (const auto& p : planes) {
            glm::vec3 positive = glm::vec3(
                p.x > 0 ? c.max.x : c.min.x,
                p.y > 0 ? c.max.y : c.min.y,
                p.z > 0 ? c.max.z : c.min.z);
            if (glm::dot(glm::vec3(p), positive) + p.w < 0.0f) {
                outside = true;
                break;
            }
        }
        if (!outside) {
            VkDrawIndirectCommand cmd{};
            cmd.vertexCount = c.vertexCount;
            cmd.instanceCount = 1;
            cmd.firstVertex = c.firstVertex;
            cmd.firstInstance = 0;
            out.push_back(cmd);
        }
    }
}

} // namespace voxelvk

#endif // VOXELVK_FRUSTUM_CULL_HPP
