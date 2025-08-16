#include "chunk_streamer.hpp"
#include <cmath>
#include <thread>
#include <chrono>

namespace voxelvk {

ChunkStreamer::ChunkStreamer(const std::string& cfgPath)
    : lod_(cfgPath) {}

ChunkStreamer::~ChunkStreamer() {
    for(auto& kv : loading_){
        if(kv.second.valid()) kv.second.wait();
    }
}

void ChunkStreamer::SetRadius(int r){
    radius_ = r;
}

void ChunkStreamer::Update(const glm::vec3& playerPos){
    glm::vec3 f = glm::floor(playerPos);
    glm::ivec3 center(f);
    std::unordered_set<int> desired;
    for(int dx=-radius_; dx<=radius_; ++dx){
        for(int dz=-radius_; dz<=radius_; ++dz){
            glm::vec2 offset(dx, dz);
            float dist = glm::length(offset);
            int id = (center.x+dx)*73856093 ^ (center.z+dz)*19349663;
            desired.insert(id);
            if(loading_.find(id)==loading_.end()){
                LoadChunkAsync(id);
            }
        }
    }
    for(auto it = loading_.begin(); it != loading_.end(); ){
        if(desired.count(it->first)==0){
            UnloadChunk(it->first);
            it = loading_.erase(it);
        }else{
            ++it;
        }
    }
}

void ChunkStreamer::LoadChunkAsync(int id){
    loading_[id] = std::async(std::launch::async, [id]{
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    });
}

void ChunkStreamer::UnloadChunk(int id){
    auto it = loading_.find(id);
    if(it != loading_.end()){
        if(it->second.valid()) it->second.wait();
    }
}

} // namespace voxelvk
