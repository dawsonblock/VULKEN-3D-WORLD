#pragma once
// Lightweight Vulkan Memory Allocator (VMA) shim for build-time integration.
// NOTE: This is a minimal compatibility layer to make the project buildable
// without vendoring the full official header. It provides only the API surface
// used in this repository. Replace with the official vk_mem_alloc.h for full
// functionality.

#include <vulkan/vulkan.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VmaAllocation_T* VmaAllocation;
typedef struct VmaAllocator_T* VmaAllocator;

typedef uint32_t VmaAllocationCreateFlags;
static const VmaAllocationCreateFlags VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT = 0x00000001u;
static const VmaAllocationCreateFlags VMA_ALLOCATION_CREATE_MAPPED_BIT                     = 0x00000002u;

typedef enum VmaMemoryUsage {
    VMA_MEMORY_USAGE_AUTO = 0,
    VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE = 1,
} VmaMemoryUsage;

typedef struct VmaVulkanFunctions {
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr   vkGetDeviceProcAddr;
    // Add more function pointers as needed for advanced use cases.
} VmaVulkanFunctions;

typedef struct VmaAllocationCreateInfo {
    VmaAllocationCreateFlags flags;
    VmaMemoryUsage usage;
} VmaAllocationCreateInfo;

typedef struct VmaAllocatorCreateInfo {
    uint32_t flags; // unused in shim
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    uint32_t vulkanApiVersion;
    const VmaVulkanFunctions* pVulkanFunctions; // optional
} VmaAllocatorCreateInfo;

// --- Shim allocator backing storage ---
struct VmaAllocator_T { VkInstance instance; VkPhysicalDevice phys; VkDevice device; };

static inline VkResult vmaCreateAllocator(const VmaAllocatorCreateInfo* info, VmaAllocator* outAlloc){
    if(!info || !outAlloc) return VK_ERROR_INITIALIZATION_FAILED;
    VmaAllocator a = (VmaAllocator)malloc(sizeof(struct VmaAllocator_T));
    if(!a) return VK_ERROR_OUT_OF_HOST_MEMORY;
    a->instance = info->instance;
    a->phys = info->physicalDevice;
    a->device = info->device;
    *outAlloc = a;
    return VK_SUCCESS;
}

static inline void vmaDestroyAllocator(VmaAllocator allocator){
    if(allocator){ free(allocator); }
}

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
    (void)image; // no-op in shim
}

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
    (void)buffer; // no-op in shim
}

static inline void vmaMapMemory(
    VmaAllocator /*allocator*/, VmaAllocation /*allocation*/, void** ppData)
{
    *ppData = NULL; // no persistent mapping in shim
}

static inline void vmaUnmapMemory(
    VmaAllocator /*allocator*/, VmaAllocation /*allocation*/)
{
    // no-op
}

#ifdef __cplusplus
}
#endif