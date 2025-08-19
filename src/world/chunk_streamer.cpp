#include "chunk_streamer.hpp"
#include <cmath>
#include <thread>
#include <chrono>

namespace voxelvk {

namespace {
constexpr int CHUNK_HASH_X_MULTIPLIER = 73856093;
constexpr int CHUNK_HASH_Z_MULTIPLIER = 19349663;
}

ChunkStreamer::ChunkStreamer(const std::string& cfgPath, std::size_t cacheSize)
    : lod_(cfgPath), cache_(cacheSize) {}

ChunkStreamer::~ChunkStreamer() {
    for (auto &kv : loading_) {
        if (kv.second.valid()) kv.second.wait();
    }
}

void ChunkStreamer::SetRadius(int r){
    radius_ = r;
}

void ChunkStreamer::Update(const glm::vec3& playerPos){
    glm::vec3 f = glm::floor(playerPos);
    glm::ivec3 center(f);
    std::unordered_set<int> desired;
    for (int dx = -radius_; dx <= radius_; ++dx) {
        for (int dz = -radius_; dz <= radius_; ++dz) {
            int id = (center.x + dx) * CHUNK_HASH_X_MULTIPLIER ^
                     (center.z + dz) * CHUNK_HASH_Z_MULTIPLIER;
            desired.insert(id);
            if (!cache_.Contains(id) && loading_.find(id) == loading_.end()) {
                LoadChunkAsync(id);
            } else {
                cache_.Touch(id);
            }
        }
    }

    for (auto it = loading_.begin(); it != loading_.end();) {
        if (it->second.wait_for(std::chrono::seconds(0)) ==
            std::future_status::ready) {
            int loadedId = it->second.get();
            if (desired.count(loadedId)) {
                if (auto ev = cache_.Insert(loadedId)) {
                    UnloadChunk(*ev);
                }
            } else {
                UnloadChunk(loadedId);
            }
            it = loading_.erase(it);
        } else {
            ++it;
        }
    }

    for (int id : cache_.Keys()) {
        if (desired.count(id) == 0) {
            cache_.Erase(id);
            UnloadChunk(id);
        }
    }
}

void ChunkStreamer::LoadChunkAsync(int id){
    loading_[id] = std::async(std::launch::async, [id] {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return id;
    });
}

void ChunkStreamer::UnloadChunk(int id){
    auto it = loading_.find(id);
    if (it != loading_.end()) {
        if (it->second.valid()) it->second.wait();
        loading_.erase(it);
    }
}

} // namespace voxelvk
