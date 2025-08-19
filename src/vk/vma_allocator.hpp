#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.h>

namespace vk {

struct AllocatedBuffer {
  VkBuffer buffer{VK_NULL_HANDLE};
  VmaAllocation allocation{};
};

VmaAllocator create_allocator(VkInstance instance,
                              VkPhysicalDevice physical_device,
                              VkDevice device);
AllocatedBuffer create_buffer(VmaAllocator allocator, VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VmaMemoryUsage memory_usage);
void destroy_buffer(VmaAllocator allocator, AllocatedBuffer &buffer);
void destroy_allocator(VmaAllocator allocator);

} // namespace vk
