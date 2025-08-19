#include "frame_graph.hpp"
#include <unordered_set>

namespace voxelvk {

void register_default_passes(FrameGraph &fg) {
    fg.add_pass("terrain", {}, [](VkCommandBuffer) {});
    fg.add_pass("shadows", {"terrain"}, [](VkCommandBuffer) {});
    fg.add_pass("lighting", {"shadows"}, [](VkCommandBuffer) {});
    fg.add_pass("post-processing", {"lighting"}, [](VkCommandBuffer) {});
}

void build_and_execute(FrameGraph &fg, VkCommandBuffer cmd) {
    std::unordered_set<std::string> done;
    std::size_t executed = 0;
    while (executed < fg.nodes.size()) {
        bool progress = false;
        for (const auto &node : fg.nodes) {
            if (done.count(node.name)) continue;
            bool ready = true;
            for (const auto &d : node.deps) {
                if (!done.count(d)) { ready = false; break; }
            }
            if (ready) {
                if (node.record) node.record(cmd);
                done.insert(node.name);
                executed++;
                progress = true;
            }
        }
        if (!progress) break; // cyclic dependency or missing dep
    }
}

} // namespace voxelvk
