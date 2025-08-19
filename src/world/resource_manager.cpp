#include "resource_manager.hpp"
#include <iostream>

namespace voxelvk {

void ResourceManager::StreamMesh(int chunkId, LODLevel level){
    loaded_[chunkId] = level;
    std::cout << "[ResourceManager] Streaming chunk " << chunkId << " LOD "
              << static_cast<int>(level) << "\n";
}

LODLevel ResourceManager::GetLOD(int chunkId) const{
    auto it = loaded_.find(chunkId);
    if(it != loaded_.end()) return it->second;
    return LODLevel::Low;
}

void ResourceManager::UnloadMesh(int chunkId){
    loaded_.erase(chunkId);
}

} // namespace voxelvk
