#include <cmath>
#include <vector>
#include <algorithm>
#include <cstddef>

extern "C" void diff_voxelize(const float* points,
                               float* occupancy,
                               float* point_grad,
                               int n_points,
                               int width,
                               int height,
                               int depth) {
    const std::size_t grid_size = static_cast<std::size_t>(width) * height * depth;
    std::fill(occupancy, occupancy + grid_size, 0.0f);
    std::fill(point_grad, point_grad + static_cast<std::size_t>(n_points) * 3, 0.0f);

    for (int i = 0; i < n_points; ++i) {
        float x = points[3 * i + 0];
        float y = points[3 * i + 1];
        float z = points[3 * i + 2];
        int ix = static_cast<int>(std::floor(x));
        int iy = static_cast<int>(std::floor(y));
        int iz = static_cast<int>(std::floor(z));
        float fx = x - static_cast<float>(ix);
        float fy = y - static_cast<float>(iy);
        float fz = z - static_cast<float>(iz);
        for (int dx = 0; dx < 2; ++dx) {
            for (int dy = 0; dy < 2; ++dy) {
                for (int dz = 0; dz < 2; ++dz) {
                    int vx = ix + dx;
                    int vy = iy + dy;
                    int vz = iz + dz;
                    if (vx < 0 || vx >= width || vy < 0 || vy >= height || vz < 0 || vz >= depth)
                        continue;
                    float wx = dx ? fx : 1.0f - fx;
                    float wy = dy ? fy : 1.0f - fy;
                    float wz = dz ? fz : 1.0f - fz;
                    float w = wx * wy * wz;
                    std::size_t idx = static_cast<std::size_t>(vx) +
                                      static_cast<std::size_t>(vy) * width +
                                      static_cast<std::size_t>(vz) * width * height;
                    occupancy[idx] += w;
                    float gx = (dx ? 1.0f : -1.0f) * wy * wz;
                    float gy = wx * (dy ? 1.0f : -1.0f) * wz;
                    float gz = wx * wy * (dz ? 1.0f : -1.0f);
                    point_grad[3 * i + 0] += gx;
                    point_grad[3 * i + 1] += gy;
        // Precompute weights and gradients for 8 combinations
        float weights[8];
        float grad_x[8];
        float grad_y[8];
        float grad_z[8];
        int idx8 = 0;
        for (int dx = 0; dx < 2; ++dx) {
            float wx = dx ? fx : 1.0f - fx;
            float gx = (dx ? 1.0f : -1.0f);
            for (int dy = 0; dy < 2; ++dy) {
                float wy = dy ? fy : 1.0f - fy;
                float gy = (dy ? 1.0f : -1.0f);
                for (int dz = 0; dz < 2; ++dz) {
                    float wz = dz ? fz : 1.0f - fz;
                    float gz = (dz ? 1.0f : -1.0f);
                    weights[idx8] = wx * wy * wz;
                    grad_x[idx8] = gx * wy * wz;
                    grad_y[idx8] = wx * gy * wz;
                    grad_z[idx8] = wx * wy * gz;
                    ++idx8;
                }
            }
        }
        // Now loop over the 8 combinations using the precomputed arrays
        idx8 = 0;
        for (int dx = 0; dx < 2; ++dx) {
            for (int dy = 0; dy < 2; ++dy) {
                for (int dz = 0; dz < 2; ++dz) {
                    int vx = ix + dx;
                    int vy = iy + dy;
                    int vz = iz + dz;
                    if (vx < 0 || vx >= width || vy < 0 || vy >= height || vz < 0 || vz >= depth)
                        continue;
                    std::size_t idx = static_cast<std::size_t>(vx) +
                                      static_cast<std::size_t>(vy) * width +
                                      static_cast<std::size_t>(vz) * width * height;
                    occupancy[idx] += weights[idx8];
                    point_grad[3 * i + 0] += grad_x[idx8];
                    point_grad[3 * i + 1] += grad_y[idx8];
                    point_grad[3 * i + 2] += grad_z[idx8];
                    ++idx8;
                }
            }
        }
    }
}

