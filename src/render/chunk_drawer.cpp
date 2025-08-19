#include "chunk_drawer.hpp"
#include <cstring>
#include <vector>

#include <glm/gtc/matrix_access.hpp>

#include "vk_mem_alloc.h"

namespace voxelvk {

namespace {

ChunkDrawer::Frustum makeFrustum(const glm::mat4 &vp) {
  ChunkDrawer::Frustum f{};
  // Left
  f.planes[0] = glm::row(vp, 3) + glm::row(vp, 0);
  // Right
  f.planes[1] = glm::row(vp, 3) - glm::row(vp, 0);
  // Bottom
  f.planes[2] = glm::row(vp, 3) + glm::row(vp, 1);
  // Top
  f.planes[3] = glm::row(vp, 3) - glm::row(vp, 1);
  // Near
  f.planes[4] = glm::row(vp, 3) + glm::row(vp, 2);
  // Far
  f.planes[5] = glm::row(vp, 3) - glm::row(vp, 2);

  for (auto &p : f.planes) {
    float invLen = 1.0f / glm::length(glm::vec3(p));
    p *= invLen;
  }
  return f;
}

bool aabbInsideFrustum(const ChunkDrawer::Frustum &fr, const glm::vec3 &mn,
                       const glm::vec3 &mx) {
  for (const auto &p : fr.planes) {
    glm::vec3 corner{p.x > 0 ? mx.x : mn.x, p.y > 0 ? mx.y : mn.y,
                     p.z > 0 ? mx.z : mn.z};
    if (glm::dot(glm::vec3(p), corner) + p.w < 0) {
      return false;
    }
  }
  return true;
}

} // namespace

bool ChunkDrawer::init(VkDevice device, VmaAllocator allocator,
                       std::size_t maxBatches) {
  device_ = device;
  allocator_ = allocator;
  capacity_ = maxBatches;

  if (maxBatches == 0) {
    return true;
  }

  VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bi.size = maxBatches * sizeof(VkDrawIndexedIndirectCommand);
  bi.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;

  VmaAllocationCreateInfo aci{};
  aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  if (vmaCreateBuffer(allocator_, &bi, &aci, &indirectBuffer_, &indirectAlloc_,
                      nullptr) != VK_SUCCESS) {
    indirectBuffer_ = VK_NULL_HANDLE;
    indirectAlloc_ = nullptr;
    return false;
  }
  return true;
}

void ChunkDrawer::destroy() {
  if (indirectBuffer_) {
    vmaDestroyBuffer(allocator_, indirectBuffer_, indirectAlloc_);
    indirectBuffer_ = VK_NULL_HANDLE;
    indirectAlloc_ = nullptr;
  }
  batches_.clear();
  occlusionResults_.clear();
}

void ChunkDrawer::setBatches(const std::vector<ChunkBatch> &batches) {
  batches_ = batches;
}

void ChunkDrawer::setOcclusionResults(const std::vector<uint64_t> &results) {
  occlusionResults_ = results;
}

ChunkDrawer::Frustum ChunkDrawer::extractFrustum(const glm::mat4 &vp) {
  return makeFrustum(vp);
}

bool ChunkDrawer::aabbInside(const Frustum &fr, const glm::vec3 &mn,
                             const glm::vec3 &mx) {
  return aabbInsideFrustum(fr, mn, mx);
}

bool ChunkDrawer::ensureCapacity(std::size_t count) {
  if (count <= capacity_) {
    return true;
  }
  destroy();
  capacity_ = count;
  VkBufferCreateInfo bi{VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
  bi.size = capacity_ * sizeof(VkDrawIndexedIndirectCommand);
  bi.usage = VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
  VmaAllocationCreateInfo aci{};
  aci.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
  return vmaCreateBuffer(allocator_, &bi, &aci, &indirectBuffer_,
                         &indirectAlloc_, nullptr) == VK_SUCCESS;
}

void ChunkDrawer::record(VkCommandBuffer cmd, const glm::mat4 &view,
                         const glm::mat4 &proj) {
  glm::mat4 vp = proj * view;
  auto fr = extractFrustum(vp);
  std::vector<VkDrawIndexedIndirectCommand> visible;
  visible.reserve(batches_.size());

  for (std::size_t i = 0; i < batches_.size(); ++i) {
    const auto &b = batches_[i];
    if (!aabbInside(fr, b.minBounds, b.maxBounds)) {
      continue;
    }
    if (useOcclusion_ && i < occlusionResults_.size() &&
        occlusionResults_[i] == 0) {
      continue;
    }
    visible.push_back(b.drawCmd);
  }

  if (visible.empty()) {
    return;
  }

  if (!ensureCapacity(visible.size())) {
    return;
  }

  void *data = nullptr;
  vmaMapMemory(allocator_, indirectAlloc_, &data);
  std::memcpy(data, visible.data(),
              visible.size() * sizeof(VkDrawIndexedIndirectCommand));
  vmaUnmapMemory(allocator_, indirectAlloc_);

  vkCmdDrawIndexedIndirect(cmd, indirectBuffer_, 0,
                           static_cast<uint32_t>(visible.size()),
                           sizeof(VkDrawIndexedIndirectCommand));
}

} // namespace voxelvk
