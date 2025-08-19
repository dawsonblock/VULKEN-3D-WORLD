#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>
#include <unordered_map>
#include "culling.hpp"

namespace voxelvk {

struct OcclusionPass {
    VkDevice device = VK_NULL_HANDLE;
    VkQueryPool queryPool = VK_NULL_HANDLE;
    bool enabled = false;

    bool init(VkDevice dev, uint32_t queryCount) {
        device = dev;
        VkQueryPoolCreateInfo ci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
        ci.queryType = VK_QUERY_TYPE_OCCLUSION;
        ci.queryCount = queryCount;
        if (vkCreateQueryPool(device, &ci, nullptr, &queryPool) == VK_SUCCESS) {
            enabled = true;
        }
        return enabled;
    }

    void destroy() {
        if (queryPool) {
            vkDestroyQueryPool(device, queryPool, nullptr);
            queryPool = VK_NULL_HANDLE;
        }
        device = VK_NULL_HANDLE;
        enabled = false;
    }

    void begin(VkCommandBuffer cmd, uint32_t query) const {
        if (!enabled) return;
        vkCmdBeginQuery(cmd, queryPool, query, 0);
    }

    void end(VkCommandBuffer cmd, uint32_t query) const {
        if (!enabled) return;
        vkCmdEndQuery(cmd, queryPool, query);
    }

    bool isVisible(uint32_t query) const {
        if (!enabled) return true;
        uint64_t result = 0;
        VkResult res = vkGetQueryPoolResults(device, queryPool, query, 1, sizeof(uint64_t), &result, sizeof(uint64_t), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
        return res == VK_SUCCESS && result > 0;
    }
};

struct ChunkInstance {
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
    uint32_t firstIndex = 0;
    uint32_t indexCount = 0;
    int32_t  vertexOffset = 0;
    uint32_t materialId = 0;
};

static std::vector<VkDrawIndexedIndirectCommand>
buildDrawCommands(const std::vector<ChunkInstance>& instances,
                   const Frustum& frustum,
                   const OcclusionPass* occ) {
    std::unordered_map<uint32_t, VkDrawIndexedIndirectCommand> batches;
    for (size_t i = 0; i < instances.size(); ++i) {
        const auto& inst = instances[i];
        if (!isBoxVisible(frustum, inst.boundsMin, inst.boundsMax)) {
            continue;
        }
        if (occ && occ->enabled && !occ->isVisible(static_cast<uint32_t>(i))) {
            continue;
        }
        auto& cmd = batches[inst.materialId];
        if (cmd.indexCount == 0) {
            cmd.indexCount = inst.indexCount;
            cmd.instanceCount = 1;
            cmd.firstIndex = inst.firstIndex;
            cmd.vertexOffset = inst.vertexOffset;
            cmd.firstInstance = 0;
        } else {
            cmd.instanceCount += 1;
        }
    }
    std::vector<VkDrawIndexedIndirectCommand> out;
    out.reserve(batches.size());
    for (auto& kv : batches) {
        out.push_back(kv.second);
    }
    return out;
}

void recordMainRenderPass(VkCommandBuffer cmd,
                          VkBuffer indirectBuffer,
                          VkDeviceSize bufferOffset,
                          const std::vector<ChunkInstance>& instances,
                          const Frustum& frustum,
                          const OcclusionPass* occ) {
    auto draws = buildDrawCommands(instances, frustum, occ);
    if (draws.empty()) return;
    // Assume indirectBuffer is host visible and already mapped by caller
    // so we simply copy commands into it before issuing draw.
    void* data = nullptr;
    // Mapping is skipped for brevity; caller should handle buffer upload.
    // vkMapMemory(..., &data);
    // memcpy(static_cast<uint8_t*>(data) + bufferOffset, draws.data(), draws.size() * sizeof(VkDrawIndexedIndirectCommand));
    // Map buffer memory, copy draw commands, and unmap before issuing indirect draw.
    void* data = nullptr;
    VkResult mapResult = vkMapMemory(device, indirectBufferMemory, bufferOffset,
                                     draws.size() * sizeof(VkDrawIndexedIndirectCommand), 0, &data);
    if (mapResult == VK_SUCCESS && data) {
        memcpy(data, draws.data(), draws.size() * sizeof(VkDrawIndexedIndirectCommand));
        vkUnmapMemory(device, indirectBufferMemory);
    } else {
        // Handle error: could not map memory
        return;
    }
    vkCmdDrawIndexedIndirect(cmd, indirectBuffer, bufferOffset,
                             static_cast<uint32_t>(draws.size()),
                             sizeof(VkDrawIndexedIndirectCommand));
}

} // namespace voxelvk

