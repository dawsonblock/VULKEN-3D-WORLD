#include "voxel_meshing.hpp"

#include <array>
#include <algorithm>

namespace voxelvk {

namespace {

// Emit a quad into the mesh buffers.  The quad is defined by the starting
// position `p`, edge vectors `du` and `dv`, and a normal direction.
void emit_quad(const std::array<int, 3>& p,
               const std::array<int, 3>& du,
               const std::array<int, 3>& dv,
               const std::array<float, 3>& normal,
               MeshBuffers& out) {
  const uint32_t base_index = static_cast<uint32_t>(out.vertices.size());

  std::array<std::array<float, 3>, 4> positions{
      std::array<float, 3>{static_cast<float>(p[0]), static_cast<float>(p[1]),
                           static_cast<float>(p[2])},
      std::array<float, 3>{static_cast<float>(p[0] + du[0]),
                           static_cast<float>(p[1] + du[1]),
                           static_cast<float>(p[2] + du[2])},
      std::array<float, 3>{static_cast<float>(p[0] + du[0] + dv[0]),
                           static_cast<float>(p[1] + du[1] + dv[1]),
                           static_cast<float>(p[2] + du[2] + dv[2])},
      std::array<float, 3>{static_cast<float>(p[0] + dv[0]),
                           static_cast<float>(p[1] + dv[1]),
                           static_cast<float>(p[2] + dv[2])}};

  for (const auto& pos : positions) {
    out.vertices.push_back(
        Vertex{pos[0], pos[1], pos[2], normal[0], normal[1], normal[2]});
  }

  // Two triangles: (0,1,2) and (0,2,3)
  out.indices.push_back(base_index + 0);
  out.indices.push_back(base_index + 1);
  out.indices.push_back(base_index + 2);
  out.indices.push_back(base_index + 0);
  out.indices.push_back(base_index + 2);
  out.indices.push_back(base_index + 3);
}

}  // namespace

// Implementation based on the algorithm described at
// https://0fps.net/2012/06/30/meshing-minecraft-part-2/.
MeshBuffers VoxelMeshing::mesh_volume_cpu(const VoxelVolume& volume) const {
  MeshBuffers mesh;
  const int dims[3] = {volume.sizeX, volume.sizeY, volume.sizeZ};
  std::vector<int> mask(static_cast<size_t>(dims[1]) * dims[2]);

  std::array<int, 3> x{};
  std::array<int, 3> q{};

  for (int d = 0; d < 3; ++d) {
    int u = (d + 1) % 3;
    int v = (d + 2) % 3;
    q[0] = q[1] = q[2] = 0;
    q[d] = 1;

    for (x[d] = -1; x[d] < dims[d];) {
      // Build mask
      int n = 0;
      for (x[v] = 0; x[v] < dims[v]; ++x[v]) {
        for (x[u] = 0; x[u] < dims[u]; ++x[u]) {
          uint16_t a = 0;
          uint16_t b = 0;
          if (x[d] >= 0) {
            a = volume.get(x[0], x[1], x[2]);
          }
          if (x[d] < dims[d] - 1) {
            b = volume.get(x[0] + q[0], x[1] + q[1], x[2] + q[2]);
          }
          if (a && b) {
            mask[n++] = 0;
          } else if (a) {
            mask[n++] = static_cast<int>(a);
          } else if (b) {
            mask[n++] = -static_cast<int>(b);
          } else {
            mask[n++] = 0;
          }
        }
      }

      ++x[d];
      n = 0;

      for (int j = 0; j < dims[v]; ++j) {
        for (int i = 0; i < dims[u];) {
          int c = mask[n];
          if (c != 0) {
            // Compute width
            int w;
            for (w = 1; i + w < dims[u] && mask[n + w] == c; ++w) {
            }
            // Compute height
            int h;
            bool done = false;
            for (h = 1; j + h < dims[v]; ++h) {
              for (int k = 0; k < w; ++k) {
                if (mask[n + k + h * dims[u]] != c) {
                  done = true;
                  break;
                }
              }
              if (done) break;
            }

            std::array<int, 3> du{};
            std::array<int, 3> dv{};
            du[u] = w;
            dv[v] = h;

            std::array<float, 3> normal{};
            normal[d] = c > 0 ? 1.0f : -1.0f;

            std::array<int, 3> p = x;
            if (c < 0) {
              p[0] += q[0];
              p[1] += q[1];
              p[2] += q[2];
            }

            emit_quad(p, du, dv, normal, mesh);

            // Zero-out mask
            for (int l = 0; l < h; ++l) {
              for (int k = 0; k < w; ++k) {
                mask[n + k + l * dims[u]] = 0;
              }
            }
            i += w;
            n += w;
          } else {
            ++i;
            ++n;
          }
        }
      }
    }
  }

  return mesh;
}

}  // namespace voxelvk

