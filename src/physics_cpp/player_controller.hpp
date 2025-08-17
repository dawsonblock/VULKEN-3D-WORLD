#pragma once
#include "capsule_voxel_sat.hpp"

struct PlayerController {
    Capsule capsule;
    Vec3 velocity{0.0f, 0.0f, 0.0f};
    bool on_ground{false};

    explicit PlayerController(const Capsule& c) : capsule(c) {}

    void integrate(float dt) {
        capsule.center += velocity * dt;
        Vec3 off = resolveCapsuleWorld(capsule);
        if (off.y > 0.0f) {
            velocity.y = 0.0f;
            on_ground = true;
        } else {
            on_ground = false;
        }
    }
};
