"""Minimal player controller used for physics tests."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from .aabb import AABB

SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(player: "PlayerController") -> float:
    """Return horizontal speed of ``player``."""
    return float(np.linalg.norm(player.vel[[0, 2]]))


@dataclass
class PlayerController:
    """Simple AABB-based player controller."""

    world: Any
    pos: NDArray[np.float32]
    vel: NDArray[np.float32]
    aabb: AABB
    input: Dict[str, int]
    gravity: float = 28.0
    max_speed: float = 11.0
    accel: float = 50.0
    air_accel: float = 10.0
    friction: float = 12.0
    jump_speed: float = 9.5
    step_height: float = 0.5
    on_ground: bool = False

    def __init__(self, world_manager: Any, spawn: NDArray[np.float32] | None = None) -> None:
        if spawn is None:
            spawn = np.array([0.0, 100.0, 0.0], dtype=np.float32)
        self.world = world_manager
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32))
        self.input = {"f": 0, "b": 0, "l": 0, "r": 0, "up": 0, "down": 0, "jump": 0, "sprint": 0}

    def set_input(self, keymap: Dict[str, int]) -> None:
        """Update input mapping from ``keymap``."""
        self.input.update({k: int(bool(v)) for k, v in keymap.items() if k in self.input})

    def update(self, dt: float, camera_forward: NDArray[np.float32], camera_right: NDArray[np.float32]) -> None:
        """Advance the controller one step using very simple kinematics."""
        wish = camera_forward * (self.input["f"] - self.input["b"]) + \
               camera_right * (self.input["r"] - self.input["l"])
        wish[1] = 0.0
        self.vel += wish * dt * self.accel
        self.pos += self.vel * dt
        self.on_ground = self.pos[1] <= 0.0
        if self.on_ground:
            self.pos[1] = 0.0
            self.vel[1] = 0.0
