#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    Vec3& operator+=(const Vec3& o) { x += o.x; y += o.y; z += o.z; return *this; }
};

inline float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
inline float length(const Vec3& v) { return std::sqrt(dot(v, v)); }

inline Vec3 normalize(const Vec3& v) {
    float len = length(v);
    return (len > 1e-9f) ? v * (1.0f / len) : Vec3(0.0f, 1.0f, 0.0f);
}

inline Vec3 clamp(const Vec3& v, const Vec3& mn, const Vec3& mx) {
    auto clampf = [](float val, float lo, float hi) {
        return val < lo ? lo : (val > hi ? hi : val);
    };
    return Vec3(
        clampf(v.x, mn.x, mx.x),
        clampf(v.y, mn.y, mx.y),
        clampf(v.z, mn.z, mx.z)
    );
}
