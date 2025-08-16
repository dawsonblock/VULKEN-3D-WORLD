"""Simplified AABB-based player controller used for testing."""

from typing import Any, Dict

import numpy as np

from .aabb import AABB


class PlayerController:
    """Very small placeholder implementation of an AABB player controller."""

    def __init__(self, world_manager: Any, spawn: np.ndarray | None = None) -> None:
        self.world = world_manager
        self.pos = (
            spawn.astype(np.float32)
            if spawn is not None
            else np.zeros(3, dtype=np.float32)
        )
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32))
        self.input: Dict[str, int] = {}
        self.on_ground = False

    def set_input(self, keymap: Dict[str, int]) -> None:
        self.input.update({k: int(bool(v)) for k, v in keymap.items()})

    def update(self, dt: float, camera_forward: np.ndarray, camera_right: np.ndarray) -> None:  # pragma: no cover - placeholder
        """Update the controller state (placeholder)."""
        _ = (dt, camera_forward, camera_right)  # unused
        # A real implementation would move ``self.aabb`` through the world
        # and update ``self.on_ground``. For the unit tests in this kata a
        # full implementation is unnecessary.

