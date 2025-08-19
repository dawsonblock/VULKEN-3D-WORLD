#include <vulkan/vulkan.h>
#include <cstdint>

namespace voxelvk {

// Push constants for a single triangle used by the voxel SAT pass.
struct TrianglePC {
    float v0[4];
    float v1[4];
    float v2[4];
};

void dispatch_voxelize_sat(VkCommandBuffer cmd,
                           VkPipeline pipeline,
                           VkPipelineLayout layout,
                           VkDescriptorSet descriptorSet,
                           const TrianglePC* triangles,
                           uint32_t triangleCount) {
    if (!cmd || !pipeline || !layout || !descriptorSet || !triangles || triangleCount == 0) {
        return;
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, 1, &descriptorSet, 0, nullptr);

    for (uint32_t i = 0; i < triangleCount; ++i) {
        vkCmdPushConstants(cmd,
                           layout,
                           VK_SHADER_STAGE_COMPUTE_BIT,
                           0,
                           sizeof(TrianglePC),
                           &triangles[i]);
        vkCmdDispatch(cmd, 1, 1, 1);
    }

}

} // namespace voxelvk

