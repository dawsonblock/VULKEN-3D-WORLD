#include "player_controller.hpp"

extern "C" {

struct CapsuleC {
    Vec3 center;
    float half_height;
    float radius;
};

int resolve_capsule_ground(CapsuleC* cap, Vec3* out_off) {
    Capsule c{cap->center, cap->half_height, cap->radius};
    Vec3 off = resolveCapsuleWorld(c);
    cap->center = c.center;
    if (out_off) *out_off = off;
    return off.y > 0.0f ? 1 : 0;
}

int capsule_box_penetration(const CapsuleC* cap, Vec3 mn, Vec3 mx,
                            Vec3* out_normal, float* out_pen) {
    Vec3 n{};
    float pen = 0.0f;
    bool hit = capsuleBoxPenetration(Capsule{cap->center, cap->half_height, cap->radius},
                                     mn, mx, n, pen);
    if (hit) {
        if (out_normal) *out_normal = n;
        if (out_pen) *out_pen = pen;
        return 1;
    }
    return 0;
}

}
