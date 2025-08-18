#include "world/svo.hpp"

namespace voxelvk {

void TraverseSvo(const std::vector<SvoNode>& nodes,
                 const std::function<void(const SvoNode&, int)>& visit,
                 uint32_t index, int level) {
    if (index >= nodes.size()) {
        return;
    }
    const SvoNode& node = nodes[index];
    visit(node, level);
    for (int i = 0; i < 8; ++i) {
        if (node.childMask & (1u << i)) {
            TraverseSvo(nodes, visit, node.children[i], level + 1);
        }
    }
}

} // namespace voxelvk

