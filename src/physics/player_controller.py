import numpy as np
from typing import Any, Dict

from .aabb import AABB


class PlayerController:
    """Simplified AABB-based player controller used only for type checking."""

    def __init__(
        self,
        world_manager: Any,
        spawn: np.ndarray = np.array([0.0, 100.0, 0.0], dtype=np.float32),
    ) -> None:
        self.world = world_manager
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos, half=np.array([0.3, 0.9, 0.3], dtype=np.float32))
        self.gravity = 28.0
        self.max_speed = 11.0
        self.accel = 50.0
        self.air_accel = 10.0
        self.friction = 12.0
        self.jump_speed = 9.5
        self.step_height = 0.5
        self.on_ground = False
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
        self.input.update({k: int(bool(v)) for k, v in keymap.items() if k in self.input})

    def update(self, dt: float, camera_forward: np.ndarray, camera_right: np.ndarray) -> None:
        """Placeholder update; real implementation omitted."""
        pass
