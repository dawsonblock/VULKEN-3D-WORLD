"""Simple capsule-based player movement controller.

The implementation is intentionally compact and only includes the pieces
required by the unit tests.  Movement is performed by integrating a velocity
vector, then resolving collisions against a voxel world using a capsule shape.
"""

from __future__ import annotations

from typing import Any, Dict

import numpy as np

from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world

# Multiplier applied to ``max_speed`` when the sprint key is active.  The same
# value is used in the tests to verify sprint behaviour.
SPRINT_SPEED_MULTIPLIER = 1.6


class PlayerControllerCapsule:
    """Capsule-based player controller used in tests.

    Parameters
    ----------
    world_manager:
        Object providing ``get_block_at_world_position`` for collision queries.
    spawn:
        Initial position of the player in world space.
    step_height, gravity, max_speed, jump_speed:
        Tuning parameters for character movement.
    """

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

        self.input: Dict[str, int] = {
            "f": 0,
            "b": 0,
            "l": 0,
            "r": 0,
            "jump": 0,
            "sprint": 0,
        }

    # ------------------------------------------------------------------ helpers
    def set_input(self, mapping: Dict[str, int]) -> None:
        """Update input state from ``mapping``.

        Only keys known to the controller are considered and values are
        converted to ``0`` or ``1``.
        """

        self.input.update({k: int(bool(v)) for k, v in mapping.items() if k in self.input})

    def _capsule(self) -> Capsule:
        """Return the player's collision capsule."""

        return Capsule(self.pos.copy(), self.half_h, self.radius)

    # ------------------------------------------------------------------ update
    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        """Advance the controller by ``dt`` seconds."""

        # Desired movement direction projected onto the ground plane.
        wish = (
            forward * (self.input["f"] - self.input["b"]) +
            right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0.0
        wl = np.linalg.norm(wish)
        if wl > 1e-6:
            wish /= wl

        target_speed = self.max_speed * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        hv = self.vel.copy()
        hv[1] = 0.0
        accel = 50.0 if self.on_ground else 10.0
        self.vel += (wish * target_speed - hv) * min(1.0, accel * dt)

        # Gravity and jumping.
        self.vel[1] -= self.g * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        # Integrate and resolve collisions.
        self.pos += self.vel * dt
        cap = self._capsule()
        off, ground = resolve_capsule_world(cap, self.world)
        if np.allclose(off, 0.0, atol=1e-6) and (
            self.input["f"]
            or self.input["l"]
            or self.input["r"]
            or self.input["b"]
        ):
            # Attempt a small step up when movement is blocked.
            cap.center[1] += self.step_height
            off2, ground2 = resolve_capsule_world(cap, self.world)
            if not np.allclose(off2, 0.0, atol=1e-6):
                ground = ground or ground2

        self.pos = cap.center
        self.on_ground = ground
        if ground and self.vel[1] < 0.0:
            self.vel[1] = 0.0

