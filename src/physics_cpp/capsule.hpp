#pragma once
#include "vec3.hpp"

struct Capsule {
    Vec3 center;
    float half_height;
    float radius;

    Vec3 segA() const { return Vec3(center.x, center.y + half_height, center.z); }
    Vec3 segB() const { return Vec3(center.x, center.y - half_height, center.z); }
};
