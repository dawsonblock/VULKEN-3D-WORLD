#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace voxelvk {

struct MeshingPass {
    bool init(VkDevice device, VkPipelineCache cache);
    void destroy(VkDevice device);

    void dispatch(VkCommandBuffer cmd,
                  VkBuffer voxels,
                  VkBuffer vertexBuffer,
                  VkBuffer indexBuffer,
                  VkBuffer counterBuffer);

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_dset_layout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipe_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
};

} // namespace voxelvk
