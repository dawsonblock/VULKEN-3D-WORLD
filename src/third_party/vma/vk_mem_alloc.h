#pragma once
// Minimal VMA stubs to satisfy compilation in this environment.
#include <vulkan/vulkan.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct VmaAllocation_T* VmaAllocation;
typedef struct VmaAllocator_T* VmaAllocator;

typedef enum VmaMemoryUsage {
    VMA_MEMORY_USAGE_AUTO = 0,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE = 1,
} VmaMemoryUsage;

typedef struct VmaAllocationCreateInfo {
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
    // We don't have device handle here; caller should destroy through Vulkan when using real VMA.
    // Stub: do nothing.
    (void)image;
}

#ifdef __cplusplus
}
#endif