"""AABB-based player controller for movement inside a voxel world."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from .aabb import AABB

SPRINT_SPEED_MULTIPLIER = 1.6


class PlayerController:
    """Axis-aligned bounding box player controller."""

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32] | None = None) -> None:
        """Create a controller at ``spawn`` within ``world_manager``."""
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

        target_speed = self.max_speed * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        accel = self.accel if self.on_ground else self.air_accel
        hv = self.vel.copy()
        hv[1] = 0.0
        self.vel += (wish * target_speed - hv) * min(1.0, accel * dt)
        if self.on_ground and wl < 1e-6:
            self.vel[0] *= max(0.0, 1.0 - self.friction * dt)
            self.vel[2] *= max(0.0, 1.0 - self.friction * dt)

        self.vel[1] -= self.gravity * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.on_ground = False
        pos_before = self.pos.copy()
        self._move_and_collide(dt)
        if np.allclose(self.pos, pos_before, atol=1e-5) and (
            self.input["f"] or self.input["l"] or self.input["r"] or self.input["b"]
        ):
            lifted = self.pos.copy()
            lifted[1] += self.step_height
            if self._can_occupy(lifted):
                self.pos = lifted
                self._move_and_collide(dt)

        self.aabb = AABB(center=self.pos, half=self.aabb.half)

    def _can_occupy(self, pos: NDArray[np.float32]) -> bool:
        """Return ``True`` if the controller can occupy ``pos``."""
        return True

    def _move_and_collide(self, dt: float) -> None:
        """Very small placeholder for movement/collision."""
        self.pos += self.vel * dt


__all__ = ["PlayerController", "SPRINT_SPEED_MULTIPLIER"]
