#pragma once
#include "vec3.hpp"

struct AABB {
    Vec3 center;
    Vec3 half;

    Vec3 min() const { return center - half; }
    Vec3 max() const { return center + half; }

    bool intersects(const AABB& other) const {
        Vec3 a_min = min();
        Vec3 a_max = max();
        Vec3 b_min = other.min();
        Vec3 b_max = other.max();
        return !(a_max.x <= b_min.x || a_min.x >= b_max.x ||
                 a_max.y <= b_min.y || a_min.y >= b_max.y ||
                 a_max.z <= b_min.z || a_min.z >= b_max.z);
    }
};
