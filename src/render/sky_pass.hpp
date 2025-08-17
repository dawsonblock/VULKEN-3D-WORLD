#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>

namespace voxelvk {

// Parameters controlling atmospheric scattering and cloud appearance.
struct SkyPushConstants {
    glm::vec3 sunDir{0.0f, 1.0f, 0.0f};
    float     time = 0.0f;
    float     atmosphereDensity = 1.0f;
    float     cloudCoverage = 0.5f;
    float     weather = 0.0f;
};

// Renders sky then overlays clouds using fullscreen shaders.
struct SkyPass {
    VkDevice              device = VK_NULL_HANDLE;
    VkPipeline            skyPipeline = VK_NULL_HANDLE;
    VkPipeline            cloudPipeline = VK_NULL_HANDLE;
    VkPipelineLayout      pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout noiseSetLayout = VK_NULL_HANDLE;

    // Initialize pipelines. Requires color attachment format of target.
    bool init(VkPhysicalDevice phys, VkDevice dev, VkFormat colorFormat);
    void destroy();

    // Record commands to draw sky then clouds (uses noiseSet for clouds).
    void record(VkCommandBuffer cmd, VkDescriptorSet noiseSet, const SkyPushConstants& pc);
};

} // namespace voxelvk

