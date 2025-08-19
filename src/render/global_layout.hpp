#pragma once
#include <vulkan/vulkan.h>

namespace voxelvk {

// Global descriptor set slots shared across passes.
enum GlobalDescriptorSet : uint32_t {
    MATERIAL_SET = 0,
    LIGHTING_SET = 1,
    CSM_SET = 2,
    IBL_SET = 3,
    GLOBAL_SET_COUNT
};

// Utility to create a pipeline layout using the global convention.
VkPipelineLayout createGlobalPipelineLayout(VkDevice device,
                                           const VkDescriptorSetLayout* setLayouts,
                                           uint32_t setLayoutCount,
                                           const VkPushConstantRange* pushes = nullptr,
                                           uint32_t pushCount = 0);

} // namespace voxelvk

