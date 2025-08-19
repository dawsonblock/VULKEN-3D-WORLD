#pragma once
#include <vulkan/vulkan.h>
#include <functional>
#include <string>
#include <vector>

namespace voxelvk {

// A simple node within the frame graph. Each node has a name, a list of
// dependencies by name and a callback that records commands for the node.
struct FrameGraphNode {
    std::string name{};
    std::vector<std::string> deps{};
    std::function<void(VkCommandBuffer)> record{};
};

// Minimal frame graph container. Nodes are registered in the order they are
// declared and executed once all their dependencies have run.
struct FrameGraph {
    std::vector<FrameGraphNode> nodes{};

    FrameGraphNode &add_pass(const std::string &name,
                             const std::vector<std::string> &deps,
                             std::function<void(VkCommandBuffer)> cb) {
        nodes.push_back({name, deps, std::move(cb)});
        return nodes.back();
    }
};

// Registers the basic rendering passes and their dependencies. Individual
// passes can still add their own custom nodes.
void register_default_passes(FrameGraph &fg);

// Topologically builds the graph (very small scale so we do it on the fly)
// and executes the callbacks in dependency order on the provided command
// buffer.
void build_and_execute(FrameGraph &fg, VkCommandBuffer cmd);

} // namespace voxelvk
