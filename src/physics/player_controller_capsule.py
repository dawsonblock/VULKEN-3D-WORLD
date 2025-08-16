

"""Capsule-based player controller using SAT collision resolution."""

from typing import Any, Dict

import numpy as np

from . import SPRINT_SPEED_MULTIPLIER
from .capsule import Capsule
from .capsule_voxel_sat import resolve_capsule_world


class PlayerControllerCapsule:
    """Capsule-based player controller.

    Parameters
    ----------
    world_manager:
        World accessor providing ``get_block_at_world_position``.
    spawn:
        Initial spawn position of the player.
    step_height, gravity, max_speed, jump_speed:
        Tuning parameters for character movement.

    Example
    -------
    >>> pc = PlayerControllerCapsule(world, np.array([0.0, 1.0, 0.0], dtype=np.float32))
    >>> forward = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    >>> right = np.array([1.0, 0.0, 0.0], dtype=np.float32)
    >>> pc.update(0.016, forward, right)
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

    def __init__(
        self,
        world_manager,
        spawn,
        step_height=0.5,
        gravity=28.0,
        max_speed=11.0,
        jump_speed=9.5,
    ):
        main
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

        self.input: Dict[str, int] = {"f":0,"b":0,"l":0,"r":0,"jump":0,"sprint":0}

    def set_input(self, m: Dict[str, int]) -> None:
        self.input.update(m)

    def _capsule(self) -> Capsule:
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt: float, forward: np.ndarray, right: np.ndarray) -> None:
        wish = (forward*(self.input["f"]-self.input["b"]) + right*(self.input["r"]-self.input["l"])); wish[1]=0
        n=np.linalg.norm(wish);  wish = wish/n if n>1e-6 else wish

        self.input = {
            "f": 0,
            "b": 0,
            "l": 0,
            "r": 0,
            "jump": 0,
            "sprint": 0,
        }

    def set_input(self, mapping):
        self.input.update(mapping)

    def _capsule(self):
        return Capsule(self.pos.copy(), self.half_h, self.radius)

    def update(self, dt, forward, right):
        wish = (
            forward * (self.input["f"] - self.input["b"])
            + right * (self.input["r"] - self.input["l"])
        )
        wish[1] = 0
        n = np.linalg.norm(wish)
        wish = wish / n if n > 1e-6 else wish
        target = self.max_speed * (
            SPRINT_SPEED_MULTIPLIER if self.input["sprint"] else 1.0
        )
        hv = self.vel.copy()
        hv[1] = 0
        self.vel += (wish * target - hv) * min(
            1.0, (50.0 if self.on_ground else 10.0) * dt
        )

        self.vel[1] -= self.g * dt
        if self.on_ground and self.input["jump"]:
            self.vel[1] = self.jump_speed
            self.on_ground = False

        self.pos += self.vel * dt
        cap = self._capsule()
        off, ground = resolve_capsule_world(cap, self.world)
        if np.allclose(off, 0.0, atol=1e-6) and (
            self.input["f"]
            or self.input["l"]
            or self.input["r"]
            or self.input["b"]
        ):
            cap.center[1] += self.step_height
            off2, ground2 = resolve_capsule_world(cap, self.world)
            if not np.allclose(off2, 0.0, atol=1e-6):
                ground = ground or ground2
        self.pos = cap.center
        self.on_ground = ground
        if ground and self.vel[1] < 0:
            self.vel[1] = 0.0
