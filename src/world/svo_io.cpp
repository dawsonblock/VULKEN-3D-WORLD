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
    if (!in || in.gcount() != sizeof(count)) {
        throw std::runtime_error("Failed to read SVO node count from file: " + path);
    }

    if (!in || count == 0 || count > MAX_SVO_NODES) {
        throw std::runtime_error("Invalid SVO node count in file: " + path);
    }

        main
    std::vector<SvoNode> nodes(count);
    for (uint32_t i = 0; i < count; ++i) {
        SvoNode& n = nodes[i];
        in.read(reinterpret_cast<char*>(&n.morton), sizeof(n.morton));
        in.read(reinterpret_cast<char*>(&n.childMask), sizeof(n.childMask));
        in.read(reinterpret_cast<char*>(&n.morton), sizeof(n.morton));

        if (in.gcount() != sizeof(n.morton)) {
            throw std::runtime_error("File truncated or corrupted while reading morton value for node " + std::to_string(i));
        }
        in.read(reinterpret_cast<char*>(&n.childMask), sizeof(n.childMask));
        if (in.gcount() != sizeof(n.childMask)) {
            throw std::runtime_error("File truncated or corrupted while reading childMask for node " + std::to_string(i));
        }
        in.read(reinterpret_cast<char*>(n.children.data()), sizeof(uint32_t) * 8);
        if (in.gcount() != sizeof(uint32_t) * 8) {
            throw std::runtime_error("File truncated or corrupted while reading children for node " + std::to_string(i));
        }

        if (!in) throw std::runtime_error("File corrupted or truncated while reading morton value for node " + std::to_string(i) + " in " + path);
        in.read(reinterpret_cast<char*>(&n.childMask), sizeof(n.childMask));
        if (!in) throw std::runtime_error("File corrupted or truncated while reading childMask for node " + std::to_string(i) + " in " + path);
        in.read(reinterpret_cast<char*>(n.children.data()), sizeof(uint32_t) * 8);
        if (!in) throw std::runtime_error("File corrupted or truncated while reading children for node " + std::to_string(i) + " in " + path);
        main
    }
    return nodes;
}

} // namespace voxelvk

