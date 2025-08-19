#include "alloc_helpers.hpp"

namespace vkx {

AllocatedBuffer create_staging_buffer(VmaAllocator allocator,
                                      VkDeviceSize size,
                                      VkBufferUsageFlags usage) {
    AllocatedBuffer out{};

    VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo info{};
    if (vmaCreateBuffer(allocator, &bci, &aci, &out.buffer, &out.allocation, &info) == VK_SUCCESS) {
        out.mapped = info.pMappedData;
    }
    return out;
}

AllocatedBuffer create_readback_buffer(VmaAllocator allocator,
                                       VkDeviceSize size,
                                       VkBufferUsageFlags usage) {
    AllocatedBuffer out{};

    VkBufferCreateInfo bci{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
    bci.size = size;
    bci.usage = usage;
    bci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo aci{};
    aci.usage = VMA_MEMORY_USAGE_AUTO;
    aci.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT |
                VMA_ALLOCATION_CREATE_MAPPED_BIT;

    VmaAllocationInfo info{};
    if (vmaCreateBuffer(allocator, &bci, &aci, &out.buffer, &out.allocation, &info) == VK_SUCCESS) {
        out.mapped = info.pMappedData;
    }
    return out;
}

VmaPool create_transient_pool(VmaAllocator allocator,
                              uint32_t memoryTypeIndex,
                              VkDeviceSize blockSize,
                              size_t maxBlockCount) {
    VmaPoolCreateInfo pci{};
    pci.memoryTypeIndex = memoryTypeIndex;
    pci.blockSize = blockSize;
    pci.maxBlockCount = maxBlockCount;
    pci.flags = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT |
                VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT;

    VmaPool pool = VK_NULL_HANDLE;
    vmaCreatePool(allocator, &pci, &pool);
    return pool;
}

} // namespace vkx
