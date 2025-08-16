"""Placeholder AABB-based player controller used for typing tests."""

from __future__ import annotations

from typing import Any, Dict, Tuple

import numpy as np

from .aabb import AABB
from .voxel_solid import is_solid


class PlayerController:
    """Minimal stub implementation of an axis-aligned player controller."""

    def __init__(
        self,
        world_manager: Any,
        spawn: np.ndarray | None = None,
    ) -> None:
        self.world = world_manager
        self.pos = (
            spawn.astype(np.float32)
            if spawn is not None
            else np.zeros(3, dtype=np.float32)
        )
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32))
        self.on_ground = False
        self.input: Dict[str, int] = {}

    def set_input(self, keymap: Dict[str, int]) -> None:
        self.input.update(keymap)

    def update(
        self, dt: float, camera_forward: np.ndarray, camera_right: np.ndarray
    ) -> None:
        """Advance the controller one step. This stub does nothing."""
        return None

