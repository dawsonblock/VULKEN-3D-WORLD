#include <cmath>

extern "C" {

struct Vec3 {
    float x;
    float y;
    float z;
};

struct CapsuleC {
    Vec3 center;
    float half_height;
    float radius;
};

int resolve_capsule_ground(CapsuleC* cap, Vec3* out_off) {
    float bottom = cap->center.y - (cap->half_height + cap->radius);
    float dy = 0.0f;
    if (bottom < 0.0f) {
        dy = -bottom;
        cap->center.y += dy;
    }
    if (out_off) {
        out_off->x = 0.0f;
        out_off->y = dy;
        out_off->z = 0.0f;
    }
    return dy > 0.0f ? 1 : 0;
}

}

