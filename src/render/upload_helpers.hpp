#pragma once
#include <vulkan/vulkan.h>

namespace voxelvk {

// Allocate a host-visible staging buffer of the given size.  The caller
// is responsible for releasing the buffer with vkDestroyBuffer and
// vkFreeMemory when no longer needed.
bool createStagingBuffer(VkPhysicalDevice phys,
                         VkDevice device,
                         VkDeviceSize size,
                         VkBuffer* outBuffer,
                         VkDeviceMemory* outMemory);

} // namespace voxelvk

