#pragma once
#include <future>
#include <unordered_map>
#include <unordered_set>
#include <glm/vec3.hpp>
#include <glm/common.hpp>

#include "lod_component.hpp"
#include "resource_manager.hpp"

namespace voxelvk {

class ChunkStreamer {
public:
    explicit ChunkStreamer(const std::string& cfgPath = "world_lod.cfg");
    ~ChunkStreamer();

    void SetRadius(int r);
    void Update(const glm::vec3& playerPos);

private:
    void LoadChunkAsync(int id);
    void LoadChunk(int id);
    void UnloadChunk(int id);

    int radius_ = 2;
    LODComponent lod_;
    ResourceManager resources_;
    std::unordered_map<int, std::future<void>> loading_;
};

} // namespace voxelvk
