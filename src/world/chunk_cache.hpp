#pragma once

#include <functional>
#include <list>
#include <memory>
#include <unordered_map>

#include "persistence.hpp"

namespace voxelvk {

class ChunkCache {
public:
    using ChunkPtr = std::shared_ptr<world::ChunkData>;

    struct Metrics {
        std::size_t hits = 0;
        std::size_t misses = 0;
        std::size_t evictions = 0;
    };

    explicit ChunkCache(std::size_t capacity);

    ChunkPtr Get(int id, const std::function<ChunkPtr(int)> &loader);
    void Clear();

    std::size_t Capacity() const { return capacity_; }
    std::size_t Size() const { return map_.size(); }
    const Metrics &GetMetrics() const { return metrics_; }

private:
    std::size_t capacity_;
    std::list<std::pair<int, ChunkPtr>> lru_;
    std::unordered_map<int, std::list<std::pair<int, ChunkPtr>>::iterator> map_;
    Metrics metrics_;
};

} // namespace voxelvk

