#pragma once

#include <vulkan/vulkan.h>
#include <cstdint>
#include <memory>

namespace voxelvk {

class ShaderWatcher;

struct VoxelFill {
    bool init(VkDevice device, VkPipelineCache cache);
    void destroy(VkDevice device);

    void dispatch(VkCommandBuffer cmd,
                  VkImageView surface_r8ui,
                  VkImageView out_r8ui,
                  uint32_t SX, uint32_t SY);

    bool reload();

private:
    bool create_pipeline();
    void destroy_pipeline();

    VkDevice m_device = VK_NULL_HANDLE;
    VkPipelineCache m_cache = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_dset_layout = VK_NULL_HANDLE;
    VkPipelineLayout m_pipe_layout = VK_NULL_HANDLE;
    VkPipeline m_pipeline = VK_NULL_HANDLE;
    VkDescriptorPool m_pool = VK_NULL_HANDLE;
    std::unique_ptr<ShaderWatcher> m_watcher;
};

} // namespace voxelvk

