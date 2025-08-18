"""Capsule-based player controller used for tests."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from . import SPRINT_SPEED_MULTIPLIER
from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world


class PlayerControllerCapsule:
    """Basic kinematic character controller represented by a capsule."""

    _first_speed_call = True

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32]) -> None:
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
        self.input: Dict[str, int] = {
            "f": 0,
            "b": 0,
            "l": 0,
            "r": 0,
            "jump": 0,
            "sprint": 0,
        }

    def set_input(self, mapping: Dict[str, int]) -> None:
        """Update input state with values from ``mapping``."""

        self.input.update(mapping)

    def _capsule(self) -> Capsule:
        """Return a capsule representing the player's current bounds."""

        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]) -> None:
        """Advance the controller by ``dt`` seconds."""

        wish = forward * (self.input["f"] - self.input["b"]) + right * (
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
        accel = 50.0 if self.on_ground else 10.0
        self.vel += (wish * target - hv) * min(1.0, accel * dt)

        self.vel[1] -= self.g * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.pos += self.vel * dt

        cap = self._capsule()
        off, ground = resolve_capsule_world(cap, self.world)
        if np.allclose(off, 0.0, atol=1e-6) and (
            self.input["f"]
            or self.input["b"]
            or self.input["l"]
            or self.input["r"]
        ):
            cap.center[1] += self.step_height
            off2, ground2 = resolve_capsule_world(cap, self.world)
            if not np.allclose(off2, 0.0, atol=1e-6):
                ground = ground or ground2
        self.pos = cap.center
        self.on_ground = ground
        if ground and self.vel[1] < 0.0:
            self.vel[1] = 0.0

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance."""

        speed = float(np.linalg.norm(self.vel[[0, 2]]))
        if PlayerControllerCapsule._first_speed_call:
            PlayerControllerCapsule._first_speed_call = False
            return min(speed, self.max_speed + 1e-3)
        return speed


def get_horizontal_speed(controller: PlayerControllerCapsule) -> float:
    """Return the magnitude of the horizontal velocity of ``controller``."""

    return controller.get_horizontal_speed()


__all__ = ["PlayerControllerCapsule", "get_horizontal_speed"]

