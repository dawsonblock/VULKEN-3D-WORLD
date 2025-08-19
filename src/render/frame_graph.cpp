#include "frame_graph.hpp"
#include <unordered_set>
#include <algorithm>

namespace voxelvk {

void FrameGraph::addPass(RenderPass* pass, std::vector<std::string> deps) {
    nodes.push_back({pass, std::move(deps)});
}

void FrameGraph::execute(VkCommandBuffer cmd) {
    std::unordered_set<std::string> executed;
    std::function<void(const Node&)> run = [&](const Node& n) {
        const std::string nm = n.pass->name();
        if (executed.count(nm)) return;
        for (const auto& depName : n.deps) {
            auto it = std::find_if(nodes.begin(), nodes.end(), [&](const Node& other) {
                return other.pass->name() == depName;
            });
            if (it != nodes.end()) run(*it);
        }
        executed.insert(nm);
        n.pass->execute(cmd);
    };

    for (const Node& n : nodes) {
        run(n);
    }
}

} // namespace voxelvk

