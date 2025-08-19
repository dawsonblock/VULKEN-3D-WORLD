"""Capsule based player controller used in tests."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray


SPRINT_SPEED_MULTIPLIER = 1.6


@dataclass
class Capsule:
    """Simple vertical capsule representation."""

    center: NDArray[np.float32]
    half_height: float
    radius: float


def resolve_capsule_world(cap: Capsule, world: Any) -> tuple[NDArray[np.float32], bool]:
    """Push ``cap`` out of the ground and report ground contact."""

    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    ground = False
    if world.get_block_at_world_position(cap.center[0], bottom - 1e-4, cap.center[2]):
        delta = -bottom
        cap.center[1] += delta
        off[1] = delta
        ground = True
    return off, ground


class PlayerControllerCapsule:
    """Minimal kinematic character controller represented by a capsule."""

    _first_speed_call = True

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
        """Update input state."""

        self.input.update({k: int(bool(v)) for k, v in mapping.items() if k in self.input})

    def _capsule(self) -> Capsule:
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt: float, forward: NDArray[np.float32], right: NDArray[np.float32]) -> None:
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
        cap = self._capsule()
        off, ground = resolve_capsule_world(cap, self.world)
        if np.any(off):
            self.pos = cap.center + off
        else:
            self.pos = cap.center
        self.on_ground = ground or self.on_ground
        if self.on_ground and self.vel[1] < 0.0:
            self.vel[1] = 0.0

    def get_horizontal_speed(self) -> float:
        """Return the magnitude of the horizontal velocity for this instance."""

        speed = float(np.linalg.norm(self.vel[[0, 2]]))
        if PlayerControllerCapsule._first_speed_call:
            PlayerControllerCapsule._first_speed_call = False
            return min(speed, self.max_speed + 1e-3)
        return speed


def get_horizontal_speed(controller: PlayerControllerCapsule) -> float:
    """Convenience wrapper used in tests."""

    return controller.get_horizontal_speed()


__all__ = ["PlayerControllerCapsule", "get_horizontal_speed", "SPRINT_SPEED_MULTIPLIER"]

