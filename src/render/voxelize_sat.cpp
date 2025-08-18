#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <algorithm>

namespace voxelvk {

struct Triangle {
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
};

struct VoxelizePush {
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;
    glm::vec3 origin;
    float voxelSize;
    glm::ivec3 minVoxel;
    glm::ivec3 gridDim;
};

void VoxelizeMeshSAT(VkCommandBuffer cmd,
                     VkPipeline pipeline,
                     VkPipelineLayout layout,
                     VkDescriptorSet occupancySet,
                     const std::vector<Triangle>& tris,
                     const glm::vec3& origin,
                     float voxelSize,
                     const glm::ivec3& gridDim) {
    for (const Triangle& t : tris) {
        glm::vec3 triMin = glm::min(t.v0, glm::min(t.v1, t.v2));
        glm::vec3 triMax = glm::max(t.v0, glm::max(t.v1, t.v2));
        glm::ivec3 minV = glm::floor((triMin - origin) / voxelSize);
        glm::ivec3 maxV = glm::floor((triMax - origin) / voxelSize);
        glm::ivec3 extent = maxV - minV + glm::ivec3(1);

        VoxelizePush pc;
        pc.v0 = t.v0;
        pc.v1 = t.v1;
        pc.v2 = t.v2;
        pc.origin = origin;
        pc.voxelSize = voxelSize;
        pc.minVoxel = minV;
        pc.gridDim = gridDim;

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &occupancySet, 0, nullptr);
        vkCmdPushConstants(cmd, layout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(VoxelizePush), &pc);
        vkCmdDispatch(cmd, static_cast<uint32_t>(extent.x),
                           static_cast<uint32_t>(extent.y),
                           static_cast<uint32_t>(extent.z));
    }
}

} // namespace voxelvk
