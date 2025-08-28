#pragma once
// Minimal VMA stubs to satisfy compilation in this environment.
#include <vulkan/vulkan.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VmaAllocation_T* VmaAllocation;
typedef struct VmaAllocator_T* VmaAllocator;

// Flags subset (stubs)
typedef enum VmaAllocationCreateFlags {
    VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT = 0x00000001,
    VMA_ALLOCATION_CREATE_MAPPED_BIT = 0x00000002,
} VmaAllocationCreateFlags;

typedef enum VmaMemoryUsage {
    VMA_MEMORY_USAGE_AUTO = 0,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE = 1,
} VmaMemoryUsage;

typedef struct VmaAllocationCreateInfo {
    VmaAllocationCreateFlags flags;
    VmaMemoryUsage usage;
} VmaAllocationCreateInfo;

static inline VkResult vmaCreateImage(
    VmaAllocator /*allocator*/, const VkImageCreateInfo* pImageCreateInfo,
    const VmaAllocationCreateInfo* /*pAllocCreateInfo*/, VkImage* pImage,
    VmaAllocation* /*pAllocation*/, void* /*pAllocationInfo*/)
{
    return vkCreateImage(VK_NULL_HANDLE, pImageCreateInfo, nullptr, pImage);
}

static inline void vmaDestroyImage(
    VmaAllocator /*allocator*/, VkImage image, VmaAllocation /*allocation*/)
{
    (void)image;
}

// Extra stubs used by csm_pass
static inline VkResult vmaCreateBuffer(
    VmaAllocator /*allocator*/, const VkBufferCreateInfo* pBufferCreateInfo,
    const VmaAllocationCreateInfo* /*pAllocCreateInfo*/, VkBuffer* pBuffer,
    VmaAllocation* /*pAllocation*/, void* /*pAllocationInfo*/)
{
    return vkCreateBuffer(VK_NULL_HANDLE, pBufferCreateInfo, nullptr, pBuffer);
}

static inline void vmaDestroyBuffer(
    VmaAllocator /*allocator*/, VkBuffer buffer, VmaAllocation /*allocation*/)
{
    (void)buffer;
}

static inline void vmaMapMemory(
    VmaAllocator /*allocator*/, VmaAllocation /*allocation*/, void** ppData)
{
    *ppData = nullptr; // no-op
}

static inline void vmaUnmapMemory(
    VmaAllocator /*allocator*/, VmaAllocation /*allocation*/)
{
}

#ifdef __cplusplus
}
#endif