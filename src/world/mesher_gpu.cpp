#include "voxel_meshing.hpp"

namespace voxelvk {

// In this repository the GPU implementation simply forwards to the CPU
// greedy mesher.  The interface is kept separate to allow future true GPU
// acceleration without changing call sites.
MeshBuffers VoxelMeshing::mesh_volume_gpu(const VoxelVolume& volume) const {
  return mesh_volume_cpu(volume);
}

MeshBuffers VoxelMeshing::mesh_volume(const VoxelVolume& volume,
                                      GPUScene& /*scene*/,
                                      VkCommandBuffer /*cmd*/) const {
  return mesh_volume_gpu(volume);
}

}  // namespace voxelvk

