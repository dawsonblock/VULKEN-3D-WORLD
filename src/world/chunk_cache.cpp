#include "chunk_cache.hpp"

namespace voxelvk {

ChunkCache::ChunkCache(std::size_t capacity)
    : capacity_(capacity) {}

bool ChunkCache::Contains(ChunkID id) const {
    return map_.find(id) != map_.end();
}

void ChunkCache::Touch(ChunkID id) {
    auto it = map_.find(id);
    if (it == map_.end()) return;
    order_.splice(order_.begin(), order_, it->second);
}

std::optional<ChunkCache::ChunkID> ChunkCache::Insert(ChunkID id) {
    auto it = map_.find(id);
    if (it != map_.end()) {
        Touch(id);
        return std::nullopt;
    }
    order_.push_front(id);
    map_[id] = order_.begin();
    if (map_.size() > capacity_) {
        auto victim = order_.back();
        order_.pop_back();
        map_.erase(victim);
        return victim;
    }
    return std::nullopt;
}

void ChunkCache::Erase(ChunkID id) {
    auto it = map_.find(id);
    if (it == map_.end()) return;
    order_.erase(it->second);
    map_.erase(it);
}

std::vector<ChunkCache::ChunkID> ChunkCache::Keys() const {
    std::vector<ChunkID> keys;
    keys.reserve(map_.size());
    for (const auto &kv : map_) keys.push_back(kv.first);
    return keys;
}

} // namespace voxelvk

