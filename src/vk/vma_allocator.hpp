#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace voxelvk {

// Create a VMA allocator bound to the given Vulkan instance, physical device, and device.
VmaAllocator create_vma_allocator(VkInstance instance,
                                 VkPhysicalDevice physical_device,
                                 VkDevice device);

// Destroy a previously created allocator.
void destroy_vma_allocator(VmaAllocator allocator);

} // namespace voxelvk

