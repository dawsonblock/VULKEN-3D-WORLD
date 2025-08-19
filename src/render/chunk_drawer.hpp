#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan.h>

struct VmaAllocator_T;
using VmaAllocator = VmaAllocator_T *;
struct VmaAllocation_T;
using VmaAllocation = VmaAllocation_T *;

namespace voxelvk {

struct ChunkBatch {
  glm::vec3 minBounds;
  glm::vec3 maxBounds;
  VkDrawIndexedIndirectCommand drawCmd{};
};

class ChunkDrawer {
public:
  bool init(VkDevice device, VmaAllocator allocator,
            std::size_t maxBatches = 0);
  void destroy();

  void setBatches(const std::vector<ChunkBatch> &batches);
  void setOcclusionResults(const std::vector<uint64_t> &results);
  void enableOcclusionCulling(bool enable) { useOcclusion_ = enable; }

  void record(VkCommandBuffer cmd, const glm::mat4 &view,
              const glm::mat4 &proj);

private:
  VkDevice device_ = VK_NULL_HANDLE;
  VmaAllocator allocator_ = nullptr;
  VkBuffer indirectBuffer_ = VK_NULL_HANDLE;
  VmaAllocation indirectAlloc_ = nullptr;
  std::size_t capacity_ = 0;

  std::vector<ChunkBatch> batches_;
  std::vector<uint64_t> occlusionResults_;
  bool useOcclusion_ = false;

  struct Frustum {
    glm::vec4 planes[6];
  };

  static Frustum extractFrustum(const glm::mat4 &vp);
  static bool aabbInside(const Frustum &fr, const glm::vec3 &mn,
                         const glm::vec3 &mx);
  bool ensureCapacity(std::size_t count);
};

} // namespace voxelvk
