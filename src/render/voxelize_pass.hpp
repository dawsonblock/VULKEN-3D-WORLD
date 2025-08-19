#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <functional>
#include <cstdint>

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace voxelvk {

using RecordVoxelDrawFn = std::function<void(VkCommandBuffer cmd, int slice)>;

struct VoxelizePass {
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

    void record(VkCommandBuffer cmd, const RecordVoxelDrawFn& drawScene);

    VkImageView getVoxelView() const { return voxelView; }

private:
    bool createImage(VkPhysicalDevice phys);
    bool createDescriptors();
    bool createPipeline(VkPhysicalDevice phys);
    uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags);
};

} // namespace voxelvk
