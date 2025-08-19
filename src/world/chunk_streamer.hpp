#pragma once
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <glm/vec3.hpp>
#include <glm/common.hpp>

#include "chunk_cache.hpp"

#include "lod_component.hpp"

namespace voxelvk {

class ChunkStreamer {
public:
    explicit ChunkStreamer(const std::string& cfgPath = "world_lod.cfg",
                          std::size_t cacheSize = 64);
    ~ChunkStreamer();

    void SetRadius(int r);
    void Update(const glm::vec3& playerPos);

private:
    void LoadChunkAsync(int id);
    void UnloadChunk(int id);

    int radius_ = 2;
    LODComponent lod_;
    ChunkCache cache_;
    std::unordered_map<int, std::future<int>> loading_;
};

} // namespace voxelvk
