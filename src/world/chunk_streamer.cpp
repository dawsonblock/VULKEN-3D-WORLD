#include "chunk_streamer.hpp"
#include <cmath>
#include <thread>
#include <chrono>
#include <unordered_set>

namespace voxelvk {

static constexpr int CHUNK_HASH_X_MULTIPLIER = 73856093;
static constexpr int CHUNK_HASH_Z_MULTIPLIER = 19349669;

ChunkStreamer::ChunkStreamer(std::size_t cacheCapacity, const std::string& cfgPath)
    : cache_(cacheCapacity), lod_(cfgPath) {}

ChunkStreamer::~ChunkStreamer() {
    for (auto& kv : loading_) {
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
            int cx = center.x + dx;
            int cz = center.z + dz;
            int id = cx*CHUNK_HASH_X_MULTIPLIER ^ cz*CHUNK_HASH_Z_MULTIPLIER;
            desired.insert(id);
            if(loading_.find(id)==loading_.end()){
                LoadChunkAsync(cx, cz, id);
            }
        }
    }
    for(auto it = loading_.begin(); it != loading_.end(); ){
        if(desired.count(it->first)==0){
            if(it->second.valid()) it->second.wait();
            it = loading_.erase(it);
        }else{
            ++it;
        }
    }
}

void ChunkStreamer::LoadChunkAsync(int cx, int cz, int id){
    loading_[id] = std::async(std::launch::async, [this, cx, cz, id]{
        cache_.Get(id, [this, cx, cz](int){
            auto chunk = GenerateChunk(cx, cz);
            MeshChunk(*chunk);
            return chunk;
        });
    });
}

std::shared_ptr<world::ChunkData> ChunkStreamer::GenerateChunk(int cx, int cz){
    // cx and cz will be used in future implementations of chunk generation.
    (void)cx; (void)cz;
    auto chunk = std::make_shared<world::ChunkData>();
    chunk->height = 0;
    chunk->size = 0;
    return chunk;
}

void ChunkStreamer::MeshChunk(const world::ChunkData& chunk){
    // 'chunk' parameter will be used in future implementations of the meshing pipeline.
    (void)chunk;
    // placeholder for meshing pipeline
}

} // namespace voxelvk

