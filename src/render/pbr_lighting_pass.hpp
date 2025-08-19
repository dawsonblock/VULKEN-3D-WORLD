#pragma once
#include <vulkan/vulkan.h>
#include "resource_manager.hpp"

namespace voxelvk {

class PBRLightingPass {
public:
    PBRLightingPass(VkDevice device, ResourceManager& resources)
        : device(device), resources(resources) {}
    void bind(VkCommandBuffer cmd, VkPipelineLayout layout, VkDescriptorSet set);
private:
    VkDevice device;
    ResourceManager& resources;
};

} // namespace voxelvk
