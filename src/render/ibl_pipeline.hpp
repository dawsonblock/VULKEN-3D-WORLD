#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <cstdint>

namespace voxelvk {

struct IBLMaps {
    VkImage       irradianceImage = VK_NULL_HANDLE;
    VkImageView   irradianceView  = VK_NULL_HANDLE;
    VkSampler     irradianceSampler = VK_NULL_HANDLE;
    VkImage       prefilterImage = VK_NULL_HANDLE;
    VkImageView   prefilterView  = VK_NULL_HANDLE;
    VkSampler     prefilterSampler = VK_NULL_HANDLE;
    uint32_t      irradianceSize = 0;
    uint32_t      prefilterSize = 0;
    uint32_t      prefilterMips = 0;
};

struct IBLDescriptor {
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkDescriptorPool      pool   = VK_NULL_HANDLE;
    VkDescriptorSet       set    = VK_NULL_HANDLE;
};

// Generates irradiance and prefiltered environment cubemaps from an input
// environment cube (envView/envSampler). The resulting maps are exposed via
// 'outMaps' and descriptor set 'outDesc' which is intended to be bound by the
// PBR shading pass.
bool GenerateIBLMaps(VkDevice device,
                     VkPhysicalDevice phys,
                     VmaAllocator allocator,
                     VkCommandPool cmdPool,
                     VkQueue queue,
                     VkImageView envView,
                     VkSampler envSampler,
                     IBLMaps& outMaps,
                     IBLDescriptor& outDesc,
                     VkFormat format = VK_FORMAT_R16G16B16A16_SFLOAT,
                     uint32_t irradianceSize = 64,
                     uint32_t prefilterSize = 256);

void DestroyIBLMaps(VkDevice device, VmaAllocator allocator, IBLMaps& maps, IBLDescriptor& desc);

} // namespace voxelvk

