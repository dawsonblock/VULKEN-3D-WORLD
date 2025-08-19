#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace vkx {

struct AllocatedBuffer {
    VkBuffer     buffer = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    void*        mapped = nullptr;
};

// Creates a host-visible buffer intended for uploading data to the GPU.
AllocatedBuffer create_staging_buffer(VmaAllocator allocator,
                                      VkDeviceSize size,
                                      VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

// Creates a host-visible buffer intended for reading data back from the GPU.
AllocatedBuffer create_readback_buffer(VmaAllocator allocator,
                                       VkDeviceSize size,
                                       VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT);

// Creates a transient allocation pool backed by VMA.
VmaPool create_transient_pool(VmaAllocator allocator,
                              uint32_t memoryTypeIndex,
                              VkDeviceSize blockSize = 0,
                              size_t maxBlockCount = 0);

// Helper for destroying a buffer created with the above helpers.
inline void destroy_buffer(VmaAllocator allocator, const AllocatedBuffer& buf) {
    if (buf.buffer != VK_NULL_HANDLE) {
        vmaDestroyBuffer(allocator, buf.buffer, buf.allocation);
    }
}

inline void destroy_pool(VmaAllocator allocator, VmaPool pool) {
    if (pool) {
        vmaDestroyPool(allocator, pool);
    }
}

} // namespace vkx
