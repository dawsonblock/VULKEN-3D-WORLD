#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include <vector>

namespace voxelvk {

struct StagingBuffer {
    VkBuffer buffer{VK_NULL_HANDLE};
    VmaAllocation allocation{nullptr};
    void* mapped{nullptr};
};

bool createStagingBuffer(VmaAllocator allocator, VkDeviceSize size, StagingBuffer& out);
void destroyStagingBuffer(VmaAllocator allocator, StagingBuffer& buf);

bool uploadBuffer(VmaAllocator allocator, VkCommandBuffer cmd, VkBuffer dst, const void* data, VkDeviceSize size);
bool readbackBuffer(VmaAllocator allocator, VkCommandBuffer cmd, VkBuffer src, void* dst, VkDeviceSize size);

bool allocateImage(VmaAllocator allocator, const VkImageCreateInfo& ci, VkImage& image, VmaAllocation& alloc);
bool allocateDeviceBuffer(VmaAllocator allocator, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& alloc);
void destroyImage(VmaAllocator allocator, VkImage& image, VmaAllocation& alloc);
void destroyBuffer(VmaAllocator allocator, VkBuffer& buffer, VmaAllocation& alloc);

class TransientBufferPool {
public:
    void init(VmaAllocator allocator);
    void reset();
    void destroy();
    bool allocate(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& allocation);
private:
    VmaAllocator allocator{nullptr};
    std::vector<std::pair<VkBuffer, VmaAllocation>> buffers;
};

} // namespace voxelvk

