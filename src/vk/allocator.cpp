#define VMA_IMPLEMENTATION
#include "vk/allocator.hpp"

namespace voxelvk {

Allocator::Allocator(VkInstance instance, VkPhysicalDevice phys, VkDevice device) {
    VmaAllocatorCreateInfo ci{};
    ci.instance = instance;
    ci.physicalDevice = phys;
    ci.device = device;
    vmaCreateAllocator(&ci, &m_allocator);
}

Allocator::~Allocator() {
    if (m_allocator) {
        vmaDestroyAllocator(m_allocator);
    }
}

VkResult Allocator::createImage(const VkImageCreateInfo* imageInfo,
                                const VmaAllocationCreateInfo* allocInfo,
                                VkImage* image, VmaAllocation* allocation) {
    return vmaCreateImage(m_allocator, imageInfo, allocInfo, image, allocation, nullptr);
}

void Allocator::destroyImage(VkImage image, VmaAllocation allocation) {
    vmaDestroyImage(m_allocator, image, allocation);
}

VkResult Allocator::createBuffer(const VkBufferCreateInfo* bufferInfo,
                                 const VmaAllocationCreateInfo* allocInfo,
                                 VkBuffer* buffer, VmaAllocation* allocation) {
    return vmaCreateBuffer(m_allocator, bufferInfo, allocInfo, buffer, allocation, nullptr);
}

void Allocator::destroyBuffer(VkBuffer buffer, VmaAllocation allocation) {
    vmaDestroyBuffer(m_allocator, buffer, allocation);
}

} // namespace voxelvk

