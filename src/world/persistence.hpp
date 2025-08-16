#pragma once

#include <cstdint>
#include <filesystem>
#include <future>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace world {

struct Chunk {
    std::pair<int, int> position;
    std::vector<std::uint8_t> voxels; // flattened xyz array
    int height{0};
    int size{0}; // length along X and Z axes
};

struct ChunkData {
    std::vector<std::uint8_t> voxels;
    int height{0};
    int size{0};
};

class ChunkStore {
public:
    ChunkStore(const std::string &root = "world_save",
               const std::string &codec = "zstd",
               bool use_rle = true,
               int threads = 4);
    ~ChunkStore();

    std::filesystem::path chunk_path(int cx, int cz) const;

    void save_chunk_sync(const Chunk &chunk);
    void save_async(const Chunk &chunk);
    std::optional<ChunkData> load_chunk(int cx, int cz);
    void wait_all();

private:
    std::filesystem::path _region_dir(int cx, int cz) const;
    std::vector<std::uint8_t> _compress(const std::vector<std::uint8_t> &data) const;
    std::vector<std::uint8_t> _decompress(const std::vector<std::uint8_t> &data) const;

    static void rle_encode(const std::vector<std::uint8_t> &arr,
                           std::vector<std::uint8_t> &vals,
                           std::vector<std::int32_t> &counts);
    static std::vector<std::uint8_t> rle_decode(const std::vector<std::uint8_t> &vals,
                                                const std::vector<std::int32_t> &counts,
                                                std::size_t total);

    std::filesystem::path root_;
    std::string codec_;
    bool use_rle_;
    std::vector<std::future<void>> futures_;
    int thread_count_;
};

} // namespace world

