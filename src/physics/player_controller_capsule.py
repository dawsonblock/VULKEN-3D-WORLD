"""Minimal capsule-based player controller used in the tests.

The implementation is intentionally lightweight; it merely supports walking,
jumping and sprinting on a very coarse voxel world.  Movement is expressed in
terms of forward/right wish vectors and relies on :func:`resolve_capsule_world`
for collision resolution.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np

from . import SPRINT_SPEED_MULTIPLIER, get_horizontal_speed
from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world


@dataclass
class _InputState:
    f: int = 0
    b: int = 0
    l: int = 0
    r: int = 0
    jump: int = 0
    sprint: int = 0


class PlayerControllerCapsule:
    """Very small kinematic character controller represented by a capsule."""

    def __init__(
        self,
        world_manager: Any,
        spawn: np.ndarray,
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
        self.input = _InputState()

    # ------------------------------------------------------------------ helpers
    def _capsule(self) -> Capsule:
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def set_input(self, mapping: Dict[str, int]) -> None:
        self.input.__dict__.update(mapping)

    # ---------------------------------------------------------------- movement
    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        # Apply gravity
        self.vel[1] -= self.g * dt

        # Desired horizontal velocity
        wish = (forward * (self.input.f - self.input.b) + right * (self.input.r - self.input.l))
        wish[1] = 0.0
        n = np.linalg.norm(wish)
        if n > 1e-6:
            wish = wish / n

        target = self.max_speed * (SPRINT_SPEED_MULTIPLIER if self.input.sprint else 1.0)
        hv = self.vel.copy()
        hv[1] = 0.0
        self.vel += (wish * target - hv) * min(1.0, (50.0 if self.on_ground else 10.0) * dt)

        # Jumping
        if self.on_ground and self.input.jump:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        # Integrate and resolve collisions
        self.pos += self.vel * dt
        cap = self._capsule()
        off, grounded = resolve_capsule_world(cap, self.world)
        self.pos = cap.center
        self.on_ground = grounded
        if grounded:
            self.vel[1] = 0.0


__all__ = ["PlayerControllerCapsule", "get_horizontal_speed", "SPRINT_SPEED_MULTIPLIER"]

