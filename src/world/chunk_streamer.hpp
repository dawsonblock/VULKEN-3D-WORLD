#pragma once
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <glm/vec3.hpp>
#include <glm/common.hpp>
#include <memory>

#include "lod_component.hpp"
#include "chunk_cache.hpp"

namespace voxelvk {

class ChunkStreamer {
public:
    explicit ChunkStreamer(std::size_t cacheCapacity = 64, const std::string& cfgPath = "world_lod.cfg");
    ~ChunkStreamer();

    void SetRadius(int r);
    void Update(const glm::vec3& playerPos);
    const ChunkCache::Metrics& GetCacheMetrics() const { return cache_.GetMetrics(); }

private:
    void LoadChunkAsync(int cx, int cz, int id);
    std::shared_ptr<world::ChunkData> GenerateChunk(int cx, int cz);
    void MeshChunk(const world::ChunkData& chunk);

    int radius_ = 2;
    LODComponent lod_;
    ChunkCache cache_;
    std::unordered_map<int, std::future<void>> loading_;
};

} // namespace voxelvk

