"""Capsule-based player controller used in tests.

This module provides a very small kinematic character controller that is
implemented entirely in Python.  The original project uses native extensions
for physics, but those are unavailable in the execution environment used by the
tests.  The implementation here mimics a tiny subset of the real behaviour so
that higher level code can be exercised.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

SPRINT_SPEED_MULTIPLIER = 1.6


@dataclass
class Capsule:
    """Simple vertical capsule used for collision checks.

    Parameters
    ----------
    center:
        Centre of the capsule in world space.
    half_height:
        Half the height of the cylindrical part of the capsule.
    radius:
        Radius of the spherical caps and the cylinder.
    """

    center: NDArray[np.float32]
    half_height: float
    radius: float


def resolve_capsule_world(cap: Capsule, world: Any) -> tuple[NDArray[np.float32], bool]:
    """Resolve collisions between ``cap`` and ``world``.

    The ``world`` object is expected to provide a
    ``get_block_at_world_position(x, y, z)`` method returning a truthy value for
    solid blocks.  The resolver only handles vertical collisions which is
    sufficient for the unit tests.  The capsule is pushed upward until it no
    longer intersects any solid blocks along the column beneath it.

    Returns
    -------
    offset:
        Translation applied to the capsule's centre to resolve the collision.
    grounded:
        ``True`` if the capsule rests on top of solid geometry.
    """

    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)

    y = float(np.floor(bottom))
    while world.get_block_at_world_position(cap.center[0], y, cap.center[2]):
        y += 1.0

    delta = y - bottom
    if delta > 0.0:
        off[1] = delta
        return off, True
    return off, False


class PlayerControllerCapsule:
    """Basic kinematic character controller represented by a capsule."""

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
        """Update the current input state."""

        self.input.update({k: v for k, v in mapping.items() if k in self.input})

    def _capsule(self) -> Capsule:
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(
        self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]
    ) -> None:
        """Advance the controller by ``dt`` seconds."""

        wish = forward * (self.input["f"] - self.input["b"]) + right * (
            self.input["r"] - self.input["l"]
        )
        wish[1] = 0.0
        n = float(np.linalg.norm(wish))
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
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.pos += self.vel * dt
        off, ground = resolve_capsule_world(self._capsule(), self.world)
        self.pos += off
        self.on_ground = ground or (self.on_ground and self.vel[1] <= 0.0)
        if self.on_ground and self.vel[1] < 0.0:
            self.vel[1] = 0.0

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance."""

        return float(np.linalg.norm(self.vel[[0, 2]]))


def get_horizontal_speed(controller: PlayerControllerCapsule) -> float:
    """Return the magnitude of the horizontal velocity of ``controller``."""

    return controller.get_horizontal_speed()


__all__ = ["PlayerControllerCapsule", "get_horizontal_speed", "SPRINT_SPEED_MULTIPLIER"]

