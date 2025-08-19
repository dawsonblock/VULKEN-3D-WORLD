#include "svo.hpp"

#include <algorithm>
#include <unordered_map>

namespace voxelvk {

static uint32_t ParentCode(uint32_t code) { return code >> 3; }
static uint32_t ChildIndex(uint32_t code) { return code & 0x7; }

std::vector<SvoNode> BuildSvo(const std::vector<uint32_t>& leaves) {
    std::vector<uint32_t> current = leaves;
    std::sort(current.begin(), current.end());

    std::vector<SvoNode> nodes;
    nodes.reserve(current.size() * 2); // rough estimate
    std::unordered_map<uint32_t, uint32_t> indexMap;

    // Create leaf nodes
    for (uint32_t code : current) {
        SvoNode n{};
        n.morton = code;
        n.childMask = 0;
        n.children.fill(UINT32_MAX);
        nodes.push_back(n);
        indexMap[code] = static_cast<uint32_t>(nodes.size() - 1);
    }

    // Aggregate bottom-up
    while (current.size() > 1) {
        std::unordered_map<uint32_t, std::vector<uint32_t>> groups;
        groups.reserve(current.size() / 8 + 1);
        for (uint32_t code : current) {
            groups[ParentCode(code)].push_back(code);
        }

        std::vector<uint32_t> parents;
        parents.reserve(groups.size());
        for (auto& kv : groups) {
            SvoNode n{};
            n.morton = kv.first;
            n.childMask = 0;
            n.children.fill(UINT32_MAX);
            for (uint32_t child : kv.second) {
                uint32_t idx = ChildIndex(child);
                n.childMask |= 1u << idx;
                n.children[idx] = indexMap[child];
            }
            nodes.push_back(n);
            uint32_t nodeIndex = static_cast<uint32_t>(nodes.size() - 1);
            indexMap[kv.first] = nodeIndex;
            parents.push_back(kv.first);
        }
        current = std::move(parents);
        std::sort(current.begin(), current.end());
        current.erase(std::unique(current.begin(), current.end()), current.end());
    }

    return nodes;
}

} // namespace voxelvk

