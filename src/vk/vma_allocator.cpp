#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

#include "vma_allocator.hpp"

namespace vk {

VmaAllocator create_allocator(VkInstance instance,
                              VkPhysicalDevice physical_device,
                              VkDevice device) {
  VmaAllocatorCreateInfo create_info{};
  create_info.instance = instance;
  create_info.physicalDevice = physical_device;
  create_info.device = device;
  VmaAllocator allocator{};
  vmaCreateAllocator(&create_info, &allocator);
  return allocator;
}

AllocatedBuffer create_buffer(VmaAllocator allocator, VkDeviceSize size,
                              VkBufferUsageFlags usage,
                              VmaMemoryUsage memory_usage) {
  VkBufferCreateInfo buffer_info{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  buffer_info.size = size;
  buffer_info.usage = usage;
  buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

  VmaAllocationCreateInfo alloc_info{};
  alloc_info.usage = memory_usage;

  AllocatedBuffer result{};
  vmaCreateBuffer(allocator, &buffer_info, &alloc_info, &result.buffer,
                  &result.allocation, nullptr);
  return result;
}

void destroy_buffer(VmaAllocator allocator, AllocatedBuffer &buffer) {
  if (buffer.buffer != VK_NULL_HANDLE && buffer.allocation != VK_NULL_HANDLE) {
    vmaDestroyBuffer(allocator, buffer.buffer, buffer.allocation);
    buffer.buffer = VK_NULL_HANDLE;
    buffer.allocation = VK_NULL_HANDLE;
  }
}

void destroy_allocator(VmaAllocator allocator) {
  if (allocator) {
    vmaDestroyAllocator(allocator);
  }
}

} // namespace vk
