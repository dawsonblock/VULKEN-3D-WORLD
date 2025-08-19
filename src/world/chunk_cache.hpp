#pragma once
#include <cstddef>
#include <list>
#include <optional>
#include <unordered_map>
#include <vector>

namespace voxelvk {

class ChunkCache {
public:
    using ChunkID = int;

    explicit ChunkCache(std::size_t capacity = 64);

    bool Contains(ChunkID id) const;
    void Touch(ChunkID id);
    std::optional<ChunkID> Insert(ChunkID id);
    void Erase(ChunkID id);
    std::vector<ChunkID> Keys() const;

private:
    std::size_t capacity_;
    std::list<ChunkID> order_;
    std::unordered_map<ChunkID, std::list<ChunkID>::iterator> map_;
};

} // namespace voxelvk

