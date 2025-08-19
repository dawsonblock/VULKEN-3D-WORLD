#pragma once
#include <cstdint>
#include <unordered_map>
#include <mutex>

namespace voxelvk {

// Simple global registry for tracking Vulkan resources.  Each resource
// is reference-counted by its raw handle value.  This allows passes to
// acquire the resources they need for a frame and release them when no
// longer used.
class ResourceRegistry {
public:
    // Access global singleton instance.
    static ResourceRegistry& instance();

    // Increase reference count for a resource handle.
    template <typename T>
    void acquire(T handle) {
        if (!handle) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        m_counts[(uint64_t)handle]++;
    }

    // Decrease reference count for a resource handle.
    template <typename T>
    void release(T handle) {
        if (!handle) return;
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = m_counts.find((uint64_t)handle);
        if (it != m_counts.end() && --(it->second) == 0) {
            m_counts.erase(it);
        }
    }

private:
    std::unordered_map<uint64_t, uint32_t> m_counts;
    std::mutex m_mutex;
};

} // namespace voxelvk

