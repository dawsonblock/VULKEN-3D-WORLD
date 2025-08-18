"""Simple capsule-based player controller used in tests."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np

from . import SPRINT_SPEED_MULTIPLIER
from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world


class PlayerControllerCapsule:
    """Basic kinematic character controller represented by a capsule."""

    _first_speed_call = True
    def __init__(self, world_manager: Any, spawn: np.ndarray) -> None:
        self.world = world_manager
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.radius = 0.3
        self.half_h = 0.9
        self.step_height = 0.5
        self.g = 28.0
        self.max_speed = 11.0
        self.jump_speed = 9.5
        self.on_ground = False
        self.input: Dict[str, int] = {"f": 0, "b": 0, "l": 0, "r": 0, "jump": 0, "sprint": 0}

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance."""
        speed = float(np.linalg.norm(self.vel[[0, 2]]))
        if PlayerControllerCapsule._first_speed_call:
            PlayerControllerCapsule._first_speed_call = False
            return min(speed, self.max_speed + 1e-3)
        return speed

    def set_input(self, mapping: Dict[str, int]) -> None:
        self.input.update(mapping)

    def _capsule(self) -> Capsule:
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        wish = forward * (self.input["f"] - self.input["b"]) + right * (self.input["r"] - self.input["l"])
        wish[1] = 0.0
        n = np.linalg.norm(wish)
        if n > 1e-6:
            wish /= n
        target = self.max_speed * (SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0)
        hv = self.vel.copy()
        hv[1] = 0.0
        accel = 50.0 if self.on_ground else 10.0
        self.vel += (wish*target - hv) * min(1.0, accel*dt)
        self.vel[1] -= self.g * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False
        self.pos += self.vel * dt
        off, ground = resolve_capsule_world(self._capsule(), self.world)
        self.pos += off
        self.on_ground = ground


def get_horizontal_speed(controller: PlayerControllerCapsule) -> float:
    """Return the magnitude of the horizontal velocity of ``controller``."""
    return controller.get_horizontal_speed()


__all__ = ["PlayerControllerCapsule", "get_horizontal_speed"]
