#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace voxelvk {

struct EnvironmentMaps {
    VkImage envImage = VK_NULL_HANDLE;
    VkImageView envView = VK_NULL_HANDLE;
    VkSampler envSampler = VK_NULL_HANDLE;
    VmaAllocation envAlloc = VK_NULL_HANDLE;
    uint32_t envSize = 0;

    VkImage irradianceImage = VK_NULL_HANDLE;
    VkImageView irradianceView = VK_NULL_HANDLE;
    VkSampler irradianceSampler = VK_NULL_HANDLE;
    VmaAllocation irradianceAlloc = VK_NULL_HANDLE;
    uint32_t irradianceSize = 0;
};

// Generates cubemap environment + irradiance map from an equirectangular texture.
bool GenerateEnvironmentMaps(VkDevice device,
                             VkPhysicalDevice phys,
                             VmaAllocator allocator,
                             VkCommandPool cmdPool,
                             VkQueue queue,
                             VkImageView equirectView,
                             VkSampler equirectSampler,
                             EnvironmentMaps& out,
                             uint32_t envSize = 256,
                             uint32_t irradianceSize = 32);

void DestroyEnvironmentMaps(VkDevice device, VmaAllocator allocator, EnvironmentMaps& env);

} // namespace voxelvk
