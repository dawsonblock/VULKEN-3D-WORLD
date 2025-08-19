#pragma once
#include <unordered_map>
#include "lod_component.hpp"

namespace voxelvk {

class ResourceManager {
public:
    void StreamMesh(int chunkId, LODLevel level);
    LODLevel GetLOD(int chunkId) const;
    void UnloadMesh(int chunkId);
private:
    std::unordered_map<int, LODLevel> loaded_;
};

} // namespace voxelvk
