#pragma once
#include <vulkan/vulkan.h>
#include <cstring>

namespace vkutil {

inline PFN_vkCmdBeginDebugUtilsLabelEXT pBeginLabel = nullptr;
inline PFN_vkCmdEndDebugUtilsLabelEXT   pEndLabel   = nullptr;

inline void init_debug_utils(VkDevice device) {
    pBeginLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(
        vkGetDeviceProcAddr(device, "vkCmdBeginDebugUtilsLabelEXT"));
    pEndLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(
        vkGetDeviceProcAddr(device, "vkCmdEndDebugUtilsLabelEXT"));
}

inline void begin_label(VkCommandBuffer cmd, const char* name, const float color[4] = nullptr) {
    if(!pBeginLabel) return;
    VkDebugUtilsLabelEXT label{VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT};
    label.pLabelName = name;
    if(color) std::memcpy(label.color, color, sizeof(float)*4);
    pBeginLabel(cmd, &label);
}

inline void end_label(VkCommandBuffer cmd) {
    if(pEndLabel) pEndLabel(cmd);
}

class GPUTimer {
public:
    bool init(VkPhysicalDevice phys, VkDevice dev) {
        device = dev;
        VkPhysicalDeviceProperties props{};
        vkGetPhysicalDeviceProperties(phys, &props);
        timestampPeriod = props.limits.timestampPeriod;
        VkQueryPoolCreateInfo ci{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
        ci.queryType = VK_QUERY_TYPE_TIMESTAMP;
        ci.queryCount = 2;
        return vkCreateQueryPool(device, &ci, nullptr, &pool) == VK_SUCCESS;
    }
    void destroy() {
        if(pool) vkDestroyQueryPool(device, pool, nullptr);
        pool = VK_NULL_HANDLE;
    }
    void reset(VkCommandBuffer cmd) {
        vkCmdResetQueryPool(cmd, pool, 0, 2);
    }
    void write_start(VkCommandBuffer cmd) {
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, pool, 0);
    }
    void write_end(VkCommandBuffer cmd) {
        vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, pool, 1);
    }
    double get_elapsed_ms() {
        uint64_t data[2]{};
        if(vkGetQueryPoolResults(device, pool, 0, 2, sizeof(data), data, sizeof(uint64_t),
                                 VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT) != VK_SUCCESS)
            return 0.0;
        uint64_t diff = data[1] - data[0];
        return static_cast<double>(diff) * timestampPeriod * 1e-6;
    }
private:
    VkDevice device = VK_NULL_HANDLE;
    VkQueryPool pool = VK_NULL_HANDLE;
    double timestampPeriod = 0.0;
};

} // namespace vkutil

