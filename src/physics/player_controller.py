"""Simplified AABB-based player controller.

This module implements a minimal kinematic character controller that moves an
axis-aligned bounding box through a voxel world. It is intentionally small and
pure Python so that it can be used in unit tests without the native physics
module that the project normally relies on.
"""

from __future__ import annotations

import logging
from typing import Any, Dict

import numpy as np
from numpy.typing import NDArray

from .aabb import AABB
from .voxel_solid import is_solid

log = logging.getLogger(__name__)

SPRINT_SPEED_MULTIPLIER = 1.6


class PlayerController:
    """Axis-aligned bounding box player controller."""

    def __init__(
        self,
        world_manager: Any,
        spawn: NDArray[np.float32] | None = None,
        *,
        gravity: float = 28.0,
        max_speed: float = 11.0,
        accel: float = 50.0,
        air_accel: float = 10.0,
        friction: float = 12.0,
        jump_speed: float = 9.5,
        step_height: float = 1.0,
    ) -> None:
        self.world = world_manager
        if spawn is None:
            spawn = np.array([0.0, 100.0, 0.0], dtype=np.float32)
        self.pos = spawn.astype(np.float32)
        self.vel = np.zeros(3, dtype=np.float32)
        self.aabb = AABB(center=self.pos.copy(), half=np.array([0.3, 0.9, 0.3], dtype=np.float32))

        self.gravity = gravity
        self.max_speed = max_speed
        self.accel = accel
        self.air_accel = air_accel
        self.friction = friction
        self.jump_speed = jump_speed
        self.step_height = step_height

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
        """Update the input mapping."""
        self.input.update({k: int(bool(v)) for k, v in keymap.items() if k in self.input})
        log.debug("Input updated: %s", self.input)

    def update(
        self,
        dt: float,
        camera_forward: NDArray[np.float32],
        camera_right: NDArray[np.float32],
    ) -> None:
        """Advance the controller one step."""

        wish = (
            camera_forward * (self.input["f"] - self.input["b"])
            + camera_right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0.0
        n = float(np.linalg.norm(wish))
        if n > 1e-6:
            wish /= n

        target_speed = self.max_speed * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        if self.on_ground:
            self.vel[[0, 2]] *= max(0.0, 1.0 - self.friction * dt)
        hv = self.vel.copy()
        hv[1] = 0.0
        accel = self.accel if self.on_ground else self.air_accel
        self.vel += (wish * target_speed - hv) * min(1.0, accel * dt)

        self.vel[1] -= self.gravity * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        ground_before = self.on_ground
        self.on_ground = False
        self._move_with_step(0, self.vel[0] * dt, ground_before)
        self._move_with_step(2, self.vel[2] * dt, ground_before)
        self._move_axis(1, self.vel[1] * dt)

        log.debug("Pos %s Vel %s Ground %s", self.pos, self.vel, self.on_ground)

    def _move_with_step(self, axis: int, amount: float, was_on_ground: bool) -> None:
        if amount == 0.0:
            return
        prev = self.pos.copy()
        collided = self._move_axis(axis, amount)
        if collided and abs(self.pos[axis] - (prev[axis] + amount)) > 1e-6 and was_on_ground:
            self.pos = prev.copy()
            self.aabb.center[:] = self.pos
            up_collided = self._move_axis(1, self.step_height)
            if not up_collided:
                second_collided = self._move_axis(axis, amount)
                self._move_axis(1, -self.step_height)
                if second_collided:
                    self.pos = prev
                    self.aabb.center[:] = self.pos
            else:
                self.pos = prev
                self.aabb.center[:] = self.pos

    def _move_axis(self, axis: int, amount: float) -> bool:
        if amount == 0.0:
            return False
        self.pos[axis] += amount
        self.aabb.center[axis] = self.pos[axis]
        collided = False
        min_i = np.floor(self.aabb.min).astype(int)
        max_i = np.floor(self.aabb.max).astype(int)
        for x in range(min_i[0], max_i[0] + 1):
            for y in range(min_i[1], max_i[1] + 1):
                for z in range(min_i[2], max_i[2] + 1):
                    block = self.world.get_block_at_world_position(x, y, z)
                    if not is_solid(block):
                        continue
                    collided = True
                    coord = x if axis == 0 else y if axis == 1 else z
                    if amount > 0:
                        self.pos[axis] = coord - self.aabb.half[axis]
                    else:
                        self.pos[axis] = coord + 1 + self.aabb.half[axis]
                    self.aabb.center[axis] = self.pos[axis]
                    self.vel[axis] = 0.0
                    if axis == 1 and amount < 0:
                        self.on_ground = True
                    break
                if collided:
                    break
            if collided:
                break
        return collided


__all__ = ["PlayerController", "SPRINT_SPEED_MULTIPLIER"]
