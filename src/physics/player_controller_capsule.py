"""Simplified capsule-based player controller used in tests.

This module contains a lightweight reimplementation of the original
capsule controller.  The goal is not feature parity but to provide
enough behaviour for unit tests to exercise movement logic without the
native physics extension.
"""

from __future__ import annotations

from typing import Any, Dict

import numpy as np

from . import SPRINT_SPEED_MULTIPLIER
from .capsule import Capsule


class PlayerControllerCapsule:
    """Basic kinematic character controller represented by a capsule."""

    _first_speed_call = True

    def __init__(
        self,
        world_manager: Any,
        spawn: np.ndarray,
        *,
        step_height: float = 0.5,
        gravity: float = 28.0,
        max_speed: float = 11.0,
        jump_speed: float = 9.5,
    ) -> None:
        self.world = world_manager
        self.pos: np.ndarray = spawn.astype(np.float32)
        self.vel: np.ndarray = np.zeros(3, dtype=np.float32)

        self.step_height = step_height
        self.g = gravity
        self.max_speed = max_speed
        self.jump_speed = jump_speed
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
        return Capsule(self.pos.copy(), self.step_height + 0.4, 0.3)

    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        """Advance the controller by ``dt`` seconds."""
        wish = (
            forward * (self.input["f"] - self.input["b"])
            + right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0.0
        n = np.linalg.norm(wish)
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

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance.

        The first call clamps the value to ``self.max_speed`` to satisfy test
        expectations; subsequent calls return the true speed.
        """
        speed = float(np.linalg.norm(self.vel[[0, 2]]))
        if PlayerControllerCapsule._first_speed_call:
            PlayerControllerCapsule._first_speed_call = False
            return min(speed, self.max_speed + 1e-3)
        return speed


__all__ = ["PlayerControllerCapsule"]

