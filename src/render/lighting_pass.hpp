#pragma once
#include <vulkan/vulkan.h>
#include <functional>

namespace voxelvk {

struct LightingPass {
    VkDevice device = VK_NULL_HANDLE;
    VkPipeline pipeline = VK_NULL_HANDLE;
    VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

    using DrawFn = std::function<void(VkCommandBuffer)>;

    bool init(VkPhysicalDevice phys, VkDevice dev, VkFormat colorFormat, VkFormat depthFormat,
              VkDescriptorSetLayout materialSet, VkDescriptorSetLayout globalSet);
    void destroy();

    void record(VkCommandBuffer cmd, VkImageView colorTarget, VkImageView depthTarget,
                VkExtent2D extent, VkDescriptorSet materialSet, VkDescriptorSet globalSet,
                const DrawFn& drawScene);
};

} // namespace voxelvk
