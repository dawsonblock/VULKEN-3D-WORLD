#pragma once
#include <glm/glm.hpp>
#include <array>

namespace voxelvk {

// Planes are represented by vec4: xyz = normal, w = distance
struct Frustum {
    std::array<glm::vec4, 6> planes; // left, right, bottom, top, near, far
};

inline Frustum extractFrustum(const glm::mat4& vp) {
    Frustum f{};
    // Left plane
    f.planes[0] = vp[3] + vp[0];
    // Right plane
    f.planes[1] = vp[3] - vp[0];
    // Bottom plane
    f.planes[2] = vp[3] + vp[1];
    // Top plane
    f.planes[3] = vp[3] - vp[1];
    // Near plane
    f.planes[4] = vp[3] + vp[2];
    // Far plane
    f.planes[5] = vp[3] - vp[2];

    // Normalize planes
    for (auto& p : f.planes) {
        float len = glm::length(glm::vec3(p));
        if (len > 0.f) {
            p /= len;
        }
    }
    return f;
}

inline bool isBoxVisible(const Frustum& f, const glm::vec3& mn, const glm::vec3& mx) {
    for (const auto& p : f.planes) {
        glm::vec3 positive = mn;
        if (p.x >= 0.f) positive.x = mx.x;
        if (p.y >= 0.f) positive.y = mx.y;
        if (p.z >= 0.f) positive.z = mx.z;
        if (glm::dot(glm::vec3(p), positive) + p.w < 0.f) {
            return false;
        }
    }
    return true;
}

} // namespace voxelvk

