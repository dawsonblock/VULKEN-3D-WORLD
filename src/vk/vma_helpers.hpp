#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace voxelvk {

struct VmaBundle {
    VmaAllocator allocator{nullptr};
};

// Create a VMA allocator from existing Vulkan handles.
// Returns true on success.
bool CreateVmaAllocator(VkInstance instance, VkPhysicalDevice phys, VkDevice device,
                        uint32_t apiVersion, VmaBundle& out);

// Destroy the allocator and clear handles.
void DestroyVmaAllocator(VmaBundle& bundle);

} // namespace voxelvk