#include "worldgen.hpp"

#include <cmath>
#include <fstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

namespace voxelvk {

using json = nlohmann::json;

WorldGenerator::WorldGenerator(const std::string& palette_file) {
    std::ifstream f(palette_file);
    if (!f.is_open()) {
        throw std::runtime_error("failed to open palette file");
    }
    json j; f >> j;
    if (j.contains("textures")) {
        for (auto& [name, id] : j["textures"].items()) {
            palette_.textures[name] = id.get<uint8_t>();
        }
    }
    if (j.contains("biomes")) {
        for (auto& [name, id] : j["biomes"].items()) {
            palette_.biomes[name] = id.get<uint8_t>();
        }
    }
}

static float Noise(int x, int z, int seed, float freq) {
    return 0.5f * (std::sin((x + seed) * freq) + std::cos((z - seed) * freq));
}

int WorldGenerator::ComputeHeight(int x, int z, int seed) const {
    float h = Noise(x, z, seed, 0.1f) * 10.0f;
    h += Noise(x, z, seed + 1337, 0.2f) * 5.0f;
    h = std::fabs(h);
    return static_cast<int>(h) % 16; // clamp to chunk height later
}

Chunk WorldGenerator::Generate(int cx, int cz, int size, int height, int seed) const {
    Chunk chunk;
    chunk.position = {cx, cz};
    chunk.size = size;
    chunk.height = height;
    chunk.voxels.assign(size * height * size, 0);
    chunk.biomes.assign(size * size, 0);

    uint8_t dirt = palette_.textures.count("dirt") ? palette_.textures.at("dirt") : 0;
    uint8_t grass = palette_.textures.count("grass") ? palette_.textures.at("grass") : dirt;
    uint8_t biome_id = palette_.biomes.count("plains") ? palette_.biomes.at("plains") : 0;

    for (int dx = 0; dx < size; ++dx) {
        for (int dz = 0; dz < size; ++dz) {
            int world_x = cx * size + dx;
            int world_z = cz * size + dz;
            int h = ComputeHeight(world_x, world_z, seed);
            int top = std::min(h, height - 1);
            for (int dy = 0; dy < top; ++dy) {
                int idx = (dy * size + dx) * size + dz;
                chunk.voxels[idx] = dirt;
            }
            int top_idx = (top * size + dx) * size + dz;
            chunk.voxels[top_idx] = grass;
            chunk.biomes[dz * size + dx] = biome_id;
        }
    }
    return chunk;
}

} // namespace voxelvk

