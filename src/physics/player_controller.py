from __future__ import annotations

"""AABB-based player controller for movement inside a voxel world."""

from typing import Any, Dict
import numpy as np
from numpy.typing import NDArray

from .aabb import AABB
from .voxel_solid import is_solid

SPRINT_SPEED_MULTIPLIER = 1.6


class PlayerController:
    """Simplified AABB-based player controller used for typing tests."""

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32] | None = None) -> None:
        self.world = world_manager
        if spawn is None:
            spawn = np.array([0.0, 100.0, 0.0], dtype=np.float32)
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32))
        self.gravity = 28.0
        self.max_speed = 11.0
        self.accel = 50.0
        self.air_accel = 10.0
        self.friction = 12.0
        self.jump_speed = 9.5
        self.step_height = 0.5
        self.on_ground = False
        self.input: Dict[str, int] = {
            "f": 0,
            "b": 0,
            "l": 0,
            "r": 0,
            "up": 0,
            "down": 0,
            "jump": 0,
            "sprint": 0,
        }

    def set_input(self, keymap: Dict[str, int]) -> None:
        """Update the input mapping."""
        self.input.update({k: int(bool(v)) for k, v in keymap.items() if k in self.input})

    def update(
        self,
        dt: float,
        camera_forward: NDArray[np.float32],
        camera_right: NDArray[np.float32],
    ) -> None:
        """Advance the controller one step using simple kinematics."""
        wish = (
            camera_forward * (self.input["f"] - self.input["b"]) +
            camera_right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0.0
        wl = float(np.linalg.norm(wish))
        if wl > 1e-6:
            wish /= wl

        target_speed = self.max_speed * (SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0)
        accel = self.accel if self.on_ground else self.air_accel
        self.vel += wish * accel * dt
        speed = float(np.linalg.norm(self.vel[[0, 2]]))
        if speed > target_speed:
            self.vel[[0, 2]] *= target_speed / max(speed, 1e-6)

        self.vel[1] -= self.gravity * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.pos += self.vel * dt
        self.aabb.center = self.pos

        # Simple ground collision
        below = self.pos[1] - self.aabb.half[1] - 1e-4
        block = self.world.get_block_at_world_position(self.pos[0], below, self.pos[2])
        if is_solid(block):
            self.pos[1] = np.ceil(self.pos[1])
            self.vel[1] = 0.0
            self.on_ground = True
        else:
            self.on_ground = False


__all__ = ["PlayerController", "SPRINT_SPEED_MULTIPLIER"]
