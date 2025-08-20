"""AABB-based player controller for movement inside a voxel world."""

from __future__ import annotations

from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from .aabb import AABB

SPRINT_SPEED_MULTIPLIER = 1.6


class PlayerController:
    """Simplified AABB-based player controller used for tests."""

    def __init__(
        self, world_manager: Any, spawn: NDArray[np.float32] | None = None
    ) -> None:
        self.world = world_manager
        if spawn is None:
            spawn = np.array([0.0, 100.0, 0.0], dtype=np.float32)
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(
            center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32)
        )
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
            camera_forward * (self.input["f"] - self.input["b"])
            + camera_right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0.0
        norm = float(np.linalg.norm(wish))
        if norm > 1e-6:
            wish /= norm
        speed = 11.0 * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        self.vel = wish * speed
        self.pos += self.vel * dt
        self.aabb.center = self.pos


__all__ = ["PlayerController", "SPRINT_SPEED_MULTIPLIER"]

