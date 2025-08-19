#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace voxelvk {

// Simple vertex layout: position and normal.
struct Vertex {
  float px, py, pz;
  float nx, ny, nz;
};

struct MeshBuffers {
  std::vector<Vertex> vertices;
  std::vector<uint32_t> indices;
};

struct VoxelVolume {
  int sizeX = 0, sizeY = 0, sizeZ = 0;
  std::vector<uint16_t> blocks;  // sizeX * sizeY * sizeZ

  [[nodiscard]] inline uint16_t get(int x, int y, int z) const {
    return blocks[static_cast<size_t>(x) +
                  static_cast<size_t>(sizeX) *
                      (static_cast<size_t>(y) +
                       static_cast<size_t>(sizeY) * static_cast<size_t>(z))];
  }
};

// Forward declarations for GPU integration.  The real engine provides these
// types, but for this repository we only need minimal stubs.
struct GPUScene {};
using VkCommandBuffer = void*;

// Voxel mesher capable of producing triangle meshes for a voxel volume using a
// greedy meshing algorithm.
class VoxelMeshing {
 public:
  // CPU implementation – generates vertex/index buffers on the host.
  [[nodiscard]] MeshBuffers mesh_volume_cpu(const VoxelVolume& volume) const;

  // GPU implementation – for now this simply mirrors the CPU algorithm but is
  // separated for future GPU acceleration.
  [[nodiscard]] MeshBuffers mesh_volume_gpu(const VoxelVolume& volume) const;

  // Convenience wrapper used by higher level code.  The returned buffers may
  // be ignored by callers that directly consume GPU output.
  [[nodiscard]] MeshBuffers mesh_volume(const VoxelVolume& volume,
                                        GPUScene& scene,
                                        VkCommandBuffer cmd) const;
};

}  // namespace voxelvk

