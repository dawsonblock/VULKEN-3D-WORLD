#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace voxelvk {

// Push constants used by the PBR rendering pass.
struct PBRPushConstants {
    glm::mat4 model{1.0f};
    glm::mat4 viewProj{1.0f};
    glm::vec3 camPos{0.0f, 0.0f, 0.0f};
};

// Forward lighting pass executing physically based rendering.
struct PBRPass {
    VkDevice         device = VK_NULL_HANDLE;
    VkPipeline       pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout materialSetLayout = VK_NULL_HANDLE;

    // Initialize graphics pipeline and descriptor layouts.
    bool init(VkPhysicalDevice phys, VkDevice dev, VkFormat colorFormat, VkFormat depthFormat);
    void destroy();

    // Record draw commands with bound material/IBL resources.
    void record(VkCommandBuffer cmd, VkDescriptorSet materialSet, const PBRPushConstants& pc, uint32_t vtxCount);
};

} // namespace voxelvk

