#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

// Forward declare VMA allocator
struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;

namespace voxelvk {

struct PBRPushConstants {
    glm::mat4 model;
    glm::mat4 viewProj;
    glm::vec3 camPos;
    float     _pad0;
    glm::vec3 lightDir;
    float     _pad1;
};

struct PBRLightingPass {
    VkDevice device = VK_NULL_HANDLE;
    VmaAllocator allocator = nullptr;

    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout setLayout = VK_NULL_HANDLE;
    VkDescriptorPool descPool = VK_NULL_HANDLE;
    VkDescriptorSet descSet = VK_NULL_HANDLE;

    bool init(VkPhysicalDevice phys, VkDevice dev, VmaAllocator alloc,
              VkFormat colorFormat, VkDescriptorSetLayout csmSetLayout);
    void destroy();

    void updateDescriptors(VkImageView albedo, VkSampler albedoSampler,
                           VkImageView metalRough, VkSampler metalRoughSampler,
                           VkImageView shadowArray, VkSampler shadowSampler);

    void record(VkCommandBuffer cmd, VkDescriptorSet csmSet,
                const PBRPushConstants& pc, uint32_t indexCount) const;
};

} // namespace voxelvk
