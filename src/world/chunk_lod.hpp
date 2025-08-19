#pragma once
#include <unordered_map>
#include <array>
#include <vector>
#include <future>
#include <memory>
#include <thread>
#include <chrono>

#include "lod_component.hpp"

namespace voxelvk {

struct Mesh {
    // Placeholder for mesh data at a particular LOD level.
};

struct ChunkLOD {
    std::array<std::shared_ptr<Mesh>, 3> meshes{};
    std::array<std::future<std::shared_ptr<Mesh>>, 3> loading{};
};

class ChunkLODCache {
public:
    // Request mesh for a chunk and LOD level. Starts async load if needed.
    std::shared_ptr<Mesh> Request(int chunkId, LODLevel level) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto& chunk = chunks_[chunkId];
        const size_t idx = static_cast<size_t>(level);
        if (!chunk.meshes[idx] && !chunk.loading[idx].valid()) {
            chunk.loading[idx] = std::async(std::launch::async, [] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return std::make_shared<Mesh>();
            });
        }
        if (chunk.loading[idx].valid() &&
            chunk.loading[idx].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
            chunk.meshes[idx] = chunk.loading[idx].get();
        }
        return chunk.meshes[idx];
    }

    // Update loading tasks and rebuild render lists.
    void Update() {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& [id, chunk] : chunks_) {
            for (size_t i = 0; i < chunk.loading.size(); ++i) {
                if (chunk.loading[i].valid() &&
                    chunk.loading[i].wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    chunk.meshes[i] = chunk.loading[i].get();
                }
            }
        }
        for (auto& list : render_lists_) list.clear();
        for (auto& [id, chunk] : chunks_) {
            for (size_t i = 0; i < chunk.meshes.size(); ++i) {
                if (chunk.meshes[i]) {
                    render_lists_[i].push_back(chunk.meshes[i]);
                }
            }
        }
    }

    const std::vector<std::shared_ptr<Mesh>>& RenderList(LODLevel level) const {
        return render_lists_[static_cast<size_t>(level)];
    }

private:
    std::unordered_map<int, ChunkLOD> chunks_{};
    std::array<std::vector<std::shared_ptr<Mesh>>, 3> render_lists_{};
};

} // namespace voxelvk

