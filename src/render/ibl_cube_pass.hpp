#pragma once
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "resource_manager.hpp"

namespace voxelvk {

// Generates irradiance and prefiltered cubemaps from an HDR skybox.
bool GenerateIBLCubeMaps(VkDevice device,
                         VkPhysicalDevice phys,
                         VmaAllocator allocator,
                         VkCommandPool cmdPool,
                         VkQueue queue,
                         VkImageView skyboxView,
                         VkSampler skyboxSampler,
                         ResourceManager& resources,
                         uint32_t envSize = 512,
                         uint32_t irradianceSize = 32,
                         uint32_t prefilterSize = 128);

} // namespace voxelvk
