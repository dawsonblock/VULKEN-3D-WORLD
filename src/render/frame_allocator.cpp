#include "frame_allocator.hpp"
#include <algorithm>

namespace voxelvk {

void FrameAllocator::beginFrame() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_offset = 0;
}

void* FrameAllocator::allocate(std::size_t size, std::size_t alignment) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::size_t current = (m_offset + alignment - 1) & ~(alignment - 1);
    if (current + size > m_buffer.size()) {
        m_buffer.resize(current + size);
    }
    void* ptr = m_buffer.data() + current;
    m_offset = current + size;
    return ptr;
}

} // namespace voxelvk

