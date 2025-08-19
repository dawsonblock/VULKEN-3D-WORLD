#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace voxelvk {

struct PBRPushConstants {
    glm::mat4 view{1.0f};
    glm::vec3 camPos{0.0f};
    float _pad0 = 0.0f;
    glm::vec3 lightDir{0.0f, -1.0f, 0.0f};
    float _pad1 = 0.0f;
};

struct PBRPass {
    VkDevice device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout materialSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout brdfSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout shadowSetLayout = VK_NULL_HANDLE;

    bool init(VkPhysicalDevice phys, VkDevice dev, VkFormat colorFormat);
    void destroy();
    void record(VkCommandBuffer cmd,
                VkDescriptorSet materialSet,
                VkDescriptorSet brdfSet,
                VkDescriptorSet shadowSet,
                const PBRPushConstants& pc);
};

} // namespace voxelvk

