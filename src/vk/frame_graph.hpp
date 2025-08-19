#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <unordered_map>
#include <vector>

#include "barrier_helpers.hpp"

namespace vkx {

struct ImageState {
    VkImageLayout layout;
    VkAccessFlags access;
    VkPipelineStageFlags stage;
};

struct BufferState {
    VkAccessFlags access;
    VkPipelineStageFlags stage;
};

struct Pass {
    std::function<void(VkCommandBuffer)> record;
    std::unordered_map<VkImage, ImageState> images;
    std::unordered_map<VkBuffer, BufferState> buffers;
};

class FrameGraph {
    std::vector<Pass> passes;

public:
    Pass &add_pass(std::function<void(VkCommandBuffer)> cb) {
        passes.push_back({cb});
        return passes.back();
    }

    void execute(VkCommandBuffer cmd) {
        std::unordered_map<VkImage, ImageState> imageStates;
        std::unordered_map<VkBuffer, BufferState> bufferStates;

        for (auto &p : passes) {
            for (auto &[img, desired] : p.images) {
                auto it = imageStates.find(img);
                if (it != imageStates.end()) {
                    if (it->second.layout != desired.layout ||
                        it->second.access != desired.access) {
                        transition_image(cmd, img, it->second.layout, desired.layout,
                                         it->second.access, desired.access,
                                         it->second.stage, desired.stage);
                        it->second = desired;
                    }
                } else {
                    imageStates[img] = desired;
                }
            }

            for (auto &[buf, desired] : p.buffers) {
                auto it = bufferStates.find(buf);
                if (it != bufferStates.end()) {
                    if (it->second.access != desired.access) {
                        transition_buffer(cmd, buf, it->second.access, desired.access,
                                          it->second.stage, desired.stage);
                        it->second = desired;
                    }
                } else {
                    bufferStates[buf] = desired;
                }
            }

            p.record(cmd);
        }
    }
};

} // namespace vkx

