"""Simplified capsule-based player controller for tests."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(controller: "PlayerControllerCapsule") -> float:
    return float(np.linalg.norm(controller.vel[[0, 2]]))


class PlayerControllerCapsule:
    """Minimal kinematic character controller represented by a capsule."""

    def __init__(
        self,
        world_manager: Any,
        spawn: NDArray[np.float32],
        step_height: float = 0.5,
        gravity: float = 28.0,
        max_speed: float = 11.0,
        jump_speed: float = 9.5,
    ) -> None:
        self.world = world_manager
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.radius = 0.3
        self.half_h = 0.9
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
        self.input.update(mapping)

    def update(
        self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]
    ) -> None:
        accel = np.zeros(3, dtype=np.float32)
        if self.input.get("f"):
            accel += forward
        if self.input.get("b"):
            accel -= forward
        if self.input.get("r"):
            accel += right
        if self.input.get("l"):
            accel -= right
        if np.linalg.norm(accel) > 0:
            accel = accel / np.linalg.norm(accel) * self.max_speed
        self.vel[[0, 2]] = accel[[0, 2]]

        if self.input.get("jump") and self.on_ground:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.vel[1] -= self.g * dt
        self.pos += self.vel * dt

        if self.pos[1] <= 0.0:
            self.pos[1] = 0.0
            self.vel[1] = 0.0
            self.on_ground = True


__all__ = [
    "PlayerControllerCapsule",
    "SPRINT_SPEED_MULTIPLIER",
    "get_horizontal_speed",
]

