"""Simple axis-aligned bounding box player controller."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray


SPRINT_SPEED_MULTIPLIER = 1.6


@dataclass
class AABB:
    """Minimal axis-aligned bounding box used by :class:`PlayerController`."""

    center: NDArray[np.float32]
    half: NDArray[np.float32]

    @property
    def min(self) -> NDArray[np.float32]:
        return self.center - self.half

    @property
    def max(self) -> NDArray[np.float32]:
        return self.center + self.half


class PlayerController:
    """Very small kinematic controller represented by an AABB."""

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
        self.on_ground = False

        self.input: Dict[str, int] = {
            "f": 0,
            "b": 0,
            "l": 0,
            "r": 0,
            "jump": 0,
            "sprint": 0,
        }

    def set_input(self, mapping: Dict[str, int]) -> None:
        """Update the input state."""

        self.input.update({k: int(bool(v)) for k, v in mapping.items() if k in self.input})

    def _is_solid(self, x: float, y: float, z: float) -> bool:
        try:
            return bool(self.world.get_block_at_world_position(x, y, z))
        except AttributeError:
            return False

    def update(
        self, dt: float, camera_forward: NDArray[np.float32], camera_right: NDArray[np.float32]
    ) -> None:
        """Advance the controller one simulation step."""

        wish = (
            camera_forward * (self.input["f"] - self.input["b"])
            + camera_right * (self.input["r"] - self.input["l"])
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

        if self.on_ground and not self.input["jump"]:
            self.vel *= max(0.0, 1.0 - self.friction * dt)

        self.vel[1] -= self.gravity * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.pos += self.vel * dt
        self.aabb.center = self.pos

        bottom = self.pos[1] - self.aabb.half[1]
        if self._is_solid(self.pos[0], bottom - 1e-4, self.pos[2]):
            self.pos[1] -= bottom
            self.vel[1] = 0.0
            self.on_ground = True
        else:
            self.on_ground = False


__all__ = ["PlayerController", "SPRINT_SPEED_MULTIPLIER"]

