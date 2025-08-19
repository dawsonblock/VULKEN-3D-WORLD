#pragma once

#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace voxelvk {

struct SvoNode {
    uint32_t morton{0};
    uint8_t childMask{0};
    std::array<uint32_t, 8> children{}; // indices into node array
};

// Build an octree from leaf Morton codes (2x2x2 grouping)
std::vector<SvoNode> BuildSvo(const std::vector<uint32_t>& leaves);

// Serialize/deserialize octree
void SaveSvo(const std::string& path, const std::vector<SvoNode>& nodes);
std::vector<SvoNode> LoadSvo(const std::string& path);

// Traverse octree (depth-first)
void TraverseSvo(const std::vector<SvoNode>& nodes,
                 const std::function<void(const SvoNode&, int)>& visit,
                 uint32_t index = 0, int level = 0);

} // namespace voxelvk

