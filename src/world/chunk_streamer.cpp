#include "chunk_streamer.hpp"
#include <cmath>
#include <thread>
#include <chrono>
#include <exception>
#include <iostream>

namespace voxelvk {

ChunkStreamer::ChunkStreamer(const std::string& cfgPath) try
    : lod_(cfgPath) {}
catch(const std::exception& e) {
    std::cerr << "ChunkStreamer initialization failed: " << e.what() << "\n";
    throw;
}

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
            int id = (center.x+dx)*CHUNK_HASH_X_MULTIPLIER ^ (center.z+dz)*CHUNK_HASH_Z_MULTIPLIER;
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
    loading_[id] = std::async(std::launch::async, [this, id]{
        this->LoadChunk(id);
    });
}

void ChunkStreamer::UnloadChunk(int id){
    auto it = loading_.find(id);
    if(it != loading_.end()){
        if(it->second.valid()) it->second.wait();
    }
}

} // namespace voxelvk
