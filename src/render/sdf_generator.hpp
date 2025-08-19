#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>
namespace voxelvk {
struct SDFVolume {
    VkImage       image = VK_NULL_HANDLE;
    VkImageView   view  = VK_NULL_HANDLE;
    VmaAllocation allocation = VK_NULL_HANDLE;
    uint32_t      width = 0, height = 0, depth = 0;
    VkFormat      format = VK_FORMAT_R16_SFLOAT;
};
bool GenerateSDF(VkDevice device,
                 VkPhysicalDevice phys,
                 VmaAllocator allocator,
                 VkCommandPool cmdPool,
                 VkQueue queue,
                 VkImage seedImage,
                 VkImageView seedView,
                 VkExtent3D extent,
                 VkFormat outFormat,
                 SDFVolume& out);
void DestroySDFVolume(VkDevice device, VmaAllocator allocator, SDFVolume& vol);
} // namespace voxelvk
