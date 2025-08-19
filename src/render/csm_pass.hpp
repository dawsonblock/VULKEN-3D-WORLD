
#pragma once
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <functional>
#include <cstdint>
#include "frame_graph.hpp"

// Forward-declare VMA
struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T*;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T*;

namespace voxelvk {

struct CSMGpuUBO {
    glm::mat4 lightVP[4];
    float     splits[4];
    int       cascadeCount;
    float     mapTexelSize;
    float     pcssMin;
    float     pcssMax;
    float     pcssSearch;
};

// Callback to record actual scene draws with a depth-only pipeline bound.
// You MUST bind vertex buffers (location=0 : vec3 position) and issue draws.
using RecordDepthDrawFn = std::function<void(VkCommandBuffer cmd, int cascadeIndex)>;

// Cascaded shadow map rendering pass.
struct CSMShadowPass : public RenderPass {
    // resources
    VkDevice        device = VK_NULL_HANDLE;
    VmaAllocator    allocator = nullptr;

    uint32_t        mapSize = 2048;
    uint32_t        cascades = 4;
    VkFormat        depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImage         depthImage = VK_NULL_HANDLE;
    VmaAllocation   depthAlloc = nullptr;
    VkImageView     depthArrayView = VK_NULL_HANDLE;
    std::vector<VkImageView> layerViews;
    VkSampler       depthSampler = VK_NULL_HANDLE;

    // pipeline
    VkPipeline              pipeline = VK_NULL_HANDLE;
    VkPipelineLayout        pipelineLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout   uboSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool        descPool = VK_NULL_HANDLE;
    VkDescriptorSet         uboSet = VK_NULL_HANDLE;

    // UBO
    VkBuffer        ubo = VK_NULL_HANDLE;
    VmaAllocation   uboAlloc = nullptr;

    // init / destroy
    bool init(VkPhysicalDevice phys, VkDevice dev, VmaAllocator alloc,
              uint32_t mapSizePx, uint32_t numCascades);
    void destroy();

    // per-frame updates
    void updateUBO(const CSMGpuUBO& data);

    // Set callback used to draw depth geometry for each cascade.
    void setDrawCallback(const RecordDepthDrawFn& fn) { drawCallback = fn; }

    // record rendering: renders each cascade layer by layer with dynamic rendering
    void record(VkCommandBuffer cmd, const RecordDepthDrawFn& drawScene);

    // RenderPass interface
    const char* name() const override { return "csm"; }
    void execute(VkCommandBuffer cmd) override {
        if (drawCallback) record(cmd, drawCallback);
    }

    // utility: get descriptor info for sampling in lighting pass
    VkImageView getDepthArrayView() const { return depthArrayView; }
    VkSampler   getDepthSampler()   const { return depthSampler; }
    void bindDepthDescriptor(VkDescriptorSet set, uint32_t binding) const;

private:
    bool createDepthArray(VkPhysicalDevice phys);
    bool createSampler();
    bool createUBO();
    bool createPipeline(VkPhysicalDevice phys);

    // helpers
    uint32_t findMemoryType(VkPhysicalDevice phys, uint32_t typeBits, VkMemoryPropertyFlags flags);

    RecordDepthDrawFn drawCallback{};
};

} // namespace voxelvk
