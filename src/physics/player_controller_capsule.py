"""Simple capsule-based player controller stub used in tests."""
from __future__ import annotations
"""Simple capsule-based player controller stub used in tests."""

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world

SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(controller: "PlayerControllerCapsule") -> float:
    """Return the horizontal speed of ``controller``."""
    return float(np.linalg.norm(controller.vel[[0, 2]]))


@dataclass
class PlayerControllerCapsule:
    world: Any
    pos: NDArray[np.float32]
    vel: NDArray[np.float32]
    on_ground: bool = False
    max_speed: float = 11.0

    def __init__(self, world: Any, spawn: NDArray[np.float32]):
        self.world = world
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)

    def set_input(self, mapping: Dict[str, int]) -> None:
        """Update input state. This stub ignores actual values."""
        pass

    def update(self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]) -> None:
        """Advance the controller by resolving collisions against the world."""
        cap = Capsule(self.pos.copy(), 0.9, 0.3)
        off, self.on_ground = resolve_capsule_world(cap, self.world)
        self.pos = cap.center
        self.pos += off


__all__ = ["PlayerControllerCapsule", "SPRINT_SPEED_MULTIPLIER", "get_horizontal_speed"]
