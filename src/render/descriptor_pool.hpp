#pragma once
#include <vulkan/vulkan.h>

namespace voxelvk {

struct PersistentDescriptors {
    VkDescriptorSetLayout layout{VK_NULL_HANDLE};
    VkDescriptorPool      pool{VK_NULL_HANDLE};
    VkDescriptorSet       set{VK_NULL_HANDLE};
};

// Create layout, pool and a single descriptor set matching the plan in docs/DESCRIPTOR_PLAN.md.
bool create_persistent_descriptors(VkDevice device, PersistentDescriptors& out);

// Destroy resources created by create_persistent_descriptors.
void destroy_persistent_descriptors(VkDevice device, PersistentDescriptors& out);

} // namespace voxelvk
