#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <array>

namespace voxelvk {

struct Palette {
    std::unordered_map<std::string, uint8_t> textures;
    std::unordered_map<std::string, uint8_t> biomes;
};

struct Chunk {
    std::array<int, 2> position{};
    std::vector<uint8_t> voxels; // size * height * size
    std::vector<uint8_t> biomes; // size * size
    int size{0};
    int height{0};
};

class WorldGenerator {
public:
    explicit WorldGenerator(const std::string& palette_file);
    Chunk Generate(int cx, int cz, int size = 16, int height = 16, int seed = 0) const;

private:
    Palette palette_{};
    int ComputeHeight(int x, int z, int seed) const;
};

} // namespace voxelvk

