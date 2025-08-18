#pragma once
#include "capsule.hpp"
#include "aabb.hpp"

inline Vec3 closestPointOnAABB(const Vec3& p, const Vec3& mn, const Vec3& mx) {
    return clamp(p, mn, mx);
}

inline Vec3 closestPointOnSegment(const Vec3& p, const Vec3& a, const Vec3& b) {
    Vec3 ab = b - a;
    float t = dot(p - a, ab) / (dot(ab, ab) + 1e-9f);
    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;
    return a + ab * t;
}

inline bool capsuleBoxPenetration(const Capsule& cap, const Vec3& mn, const Vec3& mx,
                                  Vec3& normal, float& penetration) {
    Vec3 box_center = (mn + mx) * 0.5f;
    Vec3 q_seg = closestPointOnSegment(box_center, cap.segA(), cap.segB());
    Vec3 q_box = closestPointOnAABB(q_seg, mn, mx);
    Vec3 v = q_seg - q_box;
    float dist = length(v);
    penetration = cap.radius - dist;
    if (penetration > 0.0f) {
        normal = normalize(v);
        return true;
    }
    return false;
}

inline Vec3 resolveCapsuleWorld(Capsule& cap) {
    float bottom = cap.center.y - cap.half_height - cap.radius;
    Vec3 off(0.0f, 0.0f, 0.0f);
    if (bottom < 0.0f) {
        off.y = -bottom;
        cap.center.y += off.y;
    }
    return off;
}
