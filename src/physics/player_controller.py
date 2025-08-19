"""Very small AABB-based player controller used in tests."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from . import SPRINT_SPEED_MULTIPLIER
from .aabb import AABB
from .voxel_solid import is_solid


class PlayerController:
    """Axis-aligned bounding box player controller."""

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32] | None = None) -> None:
        self.world = world_manager
        if spawn is None:
            spawn = np.array([0.0, 100.0, 0.0], dtype=np.float32)
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(self.pos, np.array([0.3, 0.9, 0.3], dtype=np.float32))
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

    def _aabb(self) -> AABB:
        return AABB(self.pos.copy(), self.aabb.half)

    def _resolve(self) -> None:
        bb = self._aabb()
        ground = False
        mn = np.floor(bb.min).astype(int)
        mx = np.floor(bb.max).astype(int)
        for y in range(mn[1] - 1, mx[1] + 2):
            for z in range(mn[2] - 1, mx[2] + 2):
                for x in range(mn[0] - 1, mx[0] + 2):
                    if not is_solid(
                        self.world.get_block_at_world_position(float(x), float(y), float(z))
                    ):
                        continue
                    block_top = y + 1.0
                    bottom = bb.min[1]
                    if bottom < block_top:
                        delta = block_top - bottom
                        self.pos[1] += delta
                        bb = bb.moved(np.array([0.0, delta, 0.0], dtype=np.float32))
                        ground = True
        self.on_ground = ground

    def update(self, dt: float, camera_forward: NDArray[np.float32], camera_right: NDArray[np.float32]) -> None:
        """Advance the controller one step."""

        wish = camera_forward * (self.input["f"] - self.input["b"]) + camera_right * (
            self.input["r"] - self.input["l"]
        )
        wish[1] = 0.0
        n = float(np.linalg.norm(wish))
        if n > 1e-6:
            wish /= n
        target = self.max_speed * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        hv = self.vel.copy()
        hv[1] = 0.0
        accel = self.accel if self.on_ground else self.air_accel
        self.vel += (wish * target - hv) * min(1.0, accel * dt)
        if self.on_ground:
            self.vel -= self.vel * min(1.0, self.friction * dt)
        self.vel[1] -= self.gravity * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False
        self.pos += self.vel * dt
        self._resolve()
        if self.on_ground and self.vel[1] < 0.0:
            self.vel[1] = 0.0


__all__ = ["PlayerController"]

