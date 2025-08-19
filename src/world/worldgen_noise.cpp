#include <vector>
#include <string>
#include <fstream>
#include <cmath>
#include <cstdint>

namespace world {

struct NoiseSettings {
    double frequency = 0.01;
    double amplitude = 1.0;
};

struct WorldgenConfig {
    NoiseSettings noise;
    std::uint8_t plains_block = 0;
    std::uint8_t mountain_block = 1;
};

WorldgenConfig load_noise_config(const std::string &path) {
    WorldgenConfig cfg;
    std::ifstream file(path);
    if (!file.is_open()) return cfg;
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    auto find_double = [&](const std::string &key, double def) {
        std::size_t pos = content.find(key);
        if (pos == std::string::npos) return def;
        pos = content.find(':', pos);
        std::size_t end = content.find_first_of(",}\n", pos + 1);
        return std::stod(content.substr(pos + 1, end - pos - 1));
    };
    auto find_int = [&](const std::string &key, int def) {
        std::size_t pos = content.find(key);
        if (pos == std::string::npos) return def;
        pos = content.find(':', pos);
        std::size_t end = content.find_first_of(",}\n", pos + 1);
        return std::stoi(content.substr(pos + 1, end - pos - 1));
    };
    cfg.noise.frequency = find_double("frequency", cfg.noise.frequency);
    cfg.noise.amplitude = find_double("amplitude", cfg.noise.amplitude);
    cfg.plains_block = static_cast<std::uint8_t>(find_int("plains_block", cfg.plains_block));
    cfg.mountain_block = static_cast<std::uint8_t>(find_int("mountain_block", cfg.mountain_block));
    return cfg;
}

void generate_chunk(int chunk_x, int chunk_z, const WorldgenConfig &cfg,
                    std::vector<std::uint8_t> &biome_out,
                    std::vector<std::uint8_t> &vox_out,
                    int size = 16) {
    biome_out.resize(size * size);
    vox_out.resize(size * size);
    for (int z = 0; z < size; ++z) {
        for (int x = 0; x < size; ++x) {
            double nx = (chunk_x * size + x) * cfg.noise.frequency;
            double nz = (chunk_z * size + z) * cfg.noise.frequency;
            double n = std::sin(nx) * std::cos(nz) * cfg.noise.amplitude;
            std::uint8_t biome = n > 0.0 ? 0 : 1;
            std::uint8_t block = biome == 0 ? cfg.plains_block : cfg.mountain_block;
            biome_out[z * size + x] = biome;
            vox_out[z * size + x] = block;
        }
    }
}

} // namespace world

