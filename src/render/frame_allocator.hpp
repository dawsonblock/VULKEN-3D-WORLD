#pragma once
#include <cstddef>
#include <vector>
#include <mutex>

namespace voxelvk {

// A simple linear allocator reset at the beginning of each frame.
class FrameAllocator {
public:
    FrameAllocator() = default;

    // Reset allocation offset for a new frame.
    void beginFrame();

    // Allocate a block of memory with the given alignment.
    void* allocate(std::size_t size, std::size_t alignment = 16);

private:
    std::vector<std::byte> m_buffer;
    std::size_t m_offset = 0;
    std::mutex m_mutex;
};

} // namespace voxelvk

