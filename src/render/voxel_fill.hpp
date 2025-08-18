#pragma once
#include <vulkan/vulkan.h>
#include <cstdint>

namespace voxelvk {

struct VoxelFill {
    bool init(VkDevice device, VkPipelineCache cache);
    void destroy(VkDevice device);

    void dispatch(VkCommandBuffer cmd,
                  VkImageView surface_r8ui,
                  VkImageView out_r8ui,
                  uint32_t SX, uint32_t SY, uint32_t SZ);

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_dset_layout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipe_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
};

} // namespace voxelvk
