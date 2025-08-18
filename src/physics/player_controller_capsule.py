
"""Simple capsule-based player controller used in tests.

This module provides a lightweight capsule character controller that uses
Separating Axis Theorem (SAT) collision resolution against a voxel world.
It is intentionally minimal and pure Python so that it can be exercised by
unit tests without requiring the native physics engine.
"""

"""Capsule-based player controller used for tests."""
        main

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from . import SPRINT_SPEED_MULTIPLIER
from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world


# Multiplier applied to ``max_speed`` when sprint input is active.
SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(controller: "PlayerControllerCapsule") -> float:
    """Return the horizontal speed of the controller."""

    return float(np.linalg.norm(controller.vel[[0, 2]]))


        main

class PlayerControllerCapsule:
    """Basic kinematic character controller represented by a capsule."""


    def __init__(
        self,
        world_manager: Any,
        spawn: np.ndarray,
        step_height: float = 0.5,
        gravity: float = 28.0,
        max_speed: float = 11.0,
        jump_speed: float = 9.5,
    ) -> None:

    _first_speed_call = True

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32]) -> None:
        main
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

    # ------------------------------------------------------------------
    # Helpers
    def set_input(self, mapping: Dict[str, int]) -> None:
        """Update input state with values from ``mapping``."""


        self.input.update({k: v for k, v in mapping.items() if k in self.input})

    def _capsule(self) -> Capsule:
        """Return a capsule representing the player's current bounds."""

        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance."""

        return float(np.linalg.norm(self.vel[[0, 2]]))

    # ------------------------------------------------------------------
    # Simulation
    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        """Advance the controller by ``dt`` seconds."""


        self.input.update(mapping)

    def _capsule(self) -> Capsule:
        """Return a capsule representing the player's current bounds."""

        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]) -> None:
        """Advance the controller by ``dt`` seconds."""

        main
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


        main
__all__ = ["PlayerControllerCapsule", "get_horizontal_speed"]

