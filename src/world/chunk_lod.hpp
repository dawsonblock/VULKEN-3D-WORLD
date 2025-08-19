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
    std::vector<float> vertices; // Vertex positions
    std::vector<uint32_t> indices; // Triangle indices
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
    ChunkLODCache(size_t pool_size = 4) : stop_pool_(false) {
        for (size_t i = 0; i < pool_size; ++i) {
            workers_.emplace_back([this] { this->Worker(); });
        }
    }
    ~ChunkLODCache() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            stop_pool_ = true;
        }
        queue_cv_.notify_all();
        for (auto& t : workers_) {
            if (t.joinable()) t.join();
        }
    }

    // Request mesh for a chunk and LOD level. Starts async load if needed.
    std::shared_ptr<Mesh> Request(int chunkId, LODLevel level) {
        auto& chunk = chunks_[chunkId];
        const size_t idx = static_cast<size_t>(level);
        if (!chunk.meshes[idx] && !chunk.loading[idx].valid()) {
            // Use packaged_task and submit to thread pool
            std::packaged_task<std::shared_ptr<Mesh>()> task([] {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                return std::make_shared<Mesh>();
            });
            chunk.loading[idx] = task.get_future();
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                tasks_.emplace(std::move(task));
            }
            queue_cv_.notify_one();
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

