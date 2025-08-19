#pragma once
#include <vulkan/vulkan.h>
#include <unordered_map>
#include <string>
#include <vector>

// Forward-declare VMA types
struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace voxelvk {

class ResourceManager {
public:
    ResourceManager() = default;

    void init(VkDevice dev, VmaAllocator alloc, uint32_t frames);
    void shutdown();
    void beginFrame(uint32_t frameIndex);
    void onSwapchainResize();

    // Wrappers around Vulkan allocation calls
    VkResult createShaderModule(const VkShaderModuleCreateInfo* ci, VkShaderModule* out) const;
    VkResult createDescriptorSetLayout(const VkDescriptorSetLayoutCreateInfo* ci, VkDescriptorSetLayout* out) const;
    VkResult createDescriptorPool(const VkDescriptorPoolCreateInfo* ci, VkDescriptorPool* out) const;
    VkResult createPipelineLayout(const VkPipelineLayoutCreateInfo* ci, VkPipelineLayout* out) const;
    VkResult createGraphicsPipeline(const VkGraphicsPipelineCreateInfo* ci, VkPipeline* out) const;
    VkResult createComputePipeline(const VkComputePipelineCreateInfo* ci, VkPipeline* out) const;
    VkResult createImageView(const VkImageViewCreateInfo* ci, VkImageView* out) const;
    VkResult createSampler(const VkSamplerCreateInfo* ci, VkSampler* out) const;

    // Staging helpers
    VkBuffer createStagingBuffer(VkDeviceSize size, void** mapped, VmaAllocation* outAlloc);
    void destroyStagingBuffer(VkBuffer buf, VmaAllocation alloc);

    // Global resource registry
    void registerBuffer(const std::string& name, VkBuffer buf);
    VkBuffer getBuffer(const std::string& name) const;
    void registerImage(const std::string& name, VkImage img);
    VkImage getImage(const std::string& name) const;

    VkDevice getDevice() const { return device; }
    VmaAllocator getAllocator() const { return allocator; }

private:
    VkDevice device{VK_NULL_HANDLE};
    VmaAllocator allocator{nullptr};
    uint32_t frameCount{0};
    uint32_t currentFrame{0};

    struct FrameAlloc {
        std::vector<VkBuffer> buffers;
        std::vector<VmaAllocation> allocs;
    };
    std::vector<FrameAlloc> frameAllocs;

    std::unordered_map<std::string, VkBuffer> globalBuffers;
    std::unordered_map<std::string, VkImage> globalImages;
};

extern ResourceManager gResourceManager;

} // namespace voxelvk

