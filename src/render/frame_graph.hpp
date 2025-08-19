#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>
#include <functional>

namespace voxelvk {

// Base interface that all render passes derive from.
class RenderPass {
public:
    virtual ~RenderPass() = default;
    // Human-readable name used for dependency tracking.
    virtual const char* name() const = 0;
    // Execute the pass, recording commands into cmd.
    virtual void execute(VkCommandBuffer cmd) = 0;
};

// Simple frame graph that schedules passes based on dependencies.
class FrameGraph {
public:
    // Add a pass with optional dependencies specified by name.
    void addPass(RenderPass* pass, std::vector<std::string> deps = {});
    // Execute all registered passes in dependency order.
    void execute(VkCommandBuffer cmd);

private:
    struct Node {
        RenderPass* pass;
        std::vector<std::string> deps;
    };
    std::vector<Node> nodes;
};

} // namespace voxelvk

