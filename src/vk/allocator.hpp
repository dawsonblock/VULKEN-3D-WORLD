#pragma once

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

namespace voxelvk {

class Allocator {
public:
    Allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
    ~Allocator();

    Allocator(const Allocator&) = delete;
    Allocator& operator=(const Allocator&) = delete;

    VkResult createImage(const VkImageCreateInfo* imageInfo,
                         const VmaAllocationCreateInfo* allocInfo,
                         VkImage* image, VmaAllocation* allocation);
    void destroyImage(VkImage image, VmaAllocation allocation);

    VkResult createBuffer(const VkBufferCreateInfo* bufferInfo,
                          const VmaAllocationCreateInfo* allocInfo,
                          VkBuffer* buffer, VmaAllocation* allocation);
    void destroyBuffer(VkBuffer buffer, VmaAllocation allocation);

private:
    VmaAllocator m_allocator{VK_NULL_HANDLE};
};

} // namespace voxelvk

