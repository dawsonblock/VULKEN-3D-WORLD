#include "chunk_cache.hpp"

namespace voxelvk {

ChunkCache::ChunkCache(std::size_t capacity) : capacity_(capacity) {}

ChunkCache::ChunkPtr ChunkCache::Get(int id, const std::function<ChunkPtr(int)> &loader) {
    auto it = map_.find(id);
    if (it != map_.end()) {
        lru_.splice(lru_.begin(), lru_, it->second);
        ++metrics_.hits;
        return it->second->second;
    }
    ++metrics_.misses;
    ChunkPtr chunk = loader(id);
    lru_.emplace_front(id, chunk);
    map_[id] = lru_.begin();
    if (map_.size() > capacity_) {
        auto last = lru_.end();
        --last;
        map_.erase(last->first);
        lru_.pop_back();
        ++metrics_.evictions;
    }
    return chunk;
}

void ChunkCache::Clear() {
    lru_.clear();
    map_.clear();
}

} // namespace voxelvk

