#include "global_layout.hpp"

namespace voxelvk {

VkPipelineLayout createGlobalPipelineLayout(VkDevice device,
                                           const VkDescriptorSetLayout* setLayouts,
                                           uint32_t setLayoutCount,
                                           const VkPushConstantRange* pushes,
                                           uint32_t pushCount){
    VkPipelineLayoutCreateInfo info{VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
    info.setLayoutCount = setLayoutCount;
    info.pSetLayouts = setLayouts;
    info.pushConstantRangeCount = pushCount;
    info.pPushConstantRanges = pushes;
    VkPipelineLayout layout = VK_NULL_HANDLE;
    vkCreatePipelineLayout(device, &info, nullptr, &layout);
    return layout;
}

} // namespace voxelvk

