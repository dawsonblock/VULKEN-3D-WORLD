#define VMA_IMPLEMENTATION
#include "vma_allocator.hpp"

namespace voxelvk {

VmaAllocator create_vma_allocator(VkInstance instance,
                                 VkPhysicalDevice physical_device,
                                 VkDevice device) {
    VmaAllocatorCreateInfo info{};
    info.instance = instance;
    info.physicalDevice = physical_device;
    info.device = device;

    VmaAllocator allocator = nullptr;
    if (vmaCreateAllocator(&info, &allocator) != VK_SUCCESS) {
        return nullptr;
    }
    return allocator;
}

void destroy_vma_allocator(VmaAllocator allocator) {
    if (allocator) {
        vmaDestroyAllocator(allocator);
    }
}

} // namespace voxelvk

