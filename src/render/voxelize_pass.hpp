#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <functional>
#include <cstdint>
#include "frame_graph.hpp"

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace voxelvk {

using RecordVoxelDrawFn = std::function<void(VkCommandBuffer cmd, int slice)>;

// Voxelizes scene geometry into a 3D texture. Implements RenderPass so it can
// be scheduled by the frame graph.
struct VoxelizePass : public RenderPass {
    VkDevice      device = VK_NULL_HANDLE;
    VmaAllocator  allocator = nullptr;
    uint32_t      dim = 0;
    VkFormat      voxelFormat = VK_FORMAT_R8_UINT;

    VkImage       voxelImage = VK_NULL_HANDLE;
    VmaAllocation voxelAlloc = nullptr;
    VkImageView   voxelView = VK_NULL_HANDLE;

    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorPool      descPool = VK_NULL_HANDLE;
    VkDescriptorSet       descSet = VK_NULL_HANDLE;

    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkPipeline       pipeline = VK_NULL_HANDLE;

    bool init(VkPhysicalDevice phys, VkDevice dev, VmaAllocator alloc, uint32_t dimension);
    void destroy();

    // Callback used to draw scene geometry during voxelization.
    void setDrawCallback(const RecordVoxelDrawFn& fn) { drawCallback = fn; }

    void record(VkCommandBuffer cmd, const RecordVoxelDrawFn& drawScene);

    // RenderPass interface
    const char* name() const override { return "voxelize"; }
    void execute(VkCommandBuffer cmd) override {
        if (drawCallback) record(cmd, drawCallback);
    }

    VkImageView getVoxelView() const { return voxelView; }

private:
    bool createImage(VkPhysicalDevice phys);
    bool createDescriptors();
    bool createPipeline(VkPhysicalDevice phys);
    uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags);

    RecordVoxelDrawFn drawCallback{};
};

} // namespace voxelvk
