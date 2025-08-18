#include "svo.hpp"

#include <fstream>
#include <stdexcept>

namespace voxelvk {

void SaveSvo(const std::string& path, const std::vector<SvoNode>& nodes) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("Failed to open file for writing: " + path);
    }
    uint32_t count = static_cast<uint32_t>(nodes.size());
    out.write(reinterpret_cast<const char*>(&count), sizeof(count));
    for (const auto& n : nodes) {
        out.write(reinterpret_cast<const char*>(&n.morton), sizeof(n.morton));
        out.write(reinterpret_cast<const char*>(&n.childMask), sizeof(n.childMask));
        out.write(reinterpret_cast<const char*>(n.children.data()), sizeof(uint32_t) * 8);
    }
}

std::vector<SvoNode> LoadSvo(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("Failed to open file for reading: " + path);
    }
    uint32_t count = 0;
    in.read(reinterpret_cast<char*>(&count), sizeof(count));
    if (!in || count == 0 || count > MAX_SVO_NODES) {
        throw std::runtime_error("Invalid SVO node count in file: " + path);
    }
    std::vector<SvoNode> nodes(count);
    for (uint32_t i = 0; i < count; ++i) {
        SvoNode& n = nodes[i];
        in.read(reinterpret_cast<char*>(&n.morton), sizeof(n.morton));
        in.read(reinterpret_cast<char*>(&n.childMask), sizeof(n.childMask));
        in.read(reinterpret_cast<char*>(n.children.data()), sizeof(uint32_t) * 8);
    }
    return nodes;
}

} // namespace voxelvk

