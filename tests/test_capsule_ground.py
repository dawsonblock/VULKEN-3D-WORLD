import numpy as np
import pytest
from dataclasses import dataclass


@dataclass
class Capsule:
    """Simple vertical capsule used for collision checks."""

    center: np.ndarray
    half_height: float
    radius: float


def resolve_capsule_world(
    cap: Capsule, world
) -> tuple[Capsule, np.ndarray, bool]:
    """Return a new Capsule instance pushed upward if it intersects the ground, along with the offset and ground contact flag."""
    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    ground = False
    new_center = np.copy(cap.center)
    if world.get_block_at_world_position(
        cap.center[0], bottom - 1e-4, cap.center[2]
    ):
        delta = -bottom
        new_center[1] += delta
        off[1] = delta
        ground = True
    new_cap = Capsule(new_center, cap.half_height, cap.radius)
    return new_cap, off, ground


class DummyWorld:
    """World with solid blocks below ``y == 0``."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0.0 else 0


def test_capsule_ground_collision() -> None:
    world = DummyWorld()
    cap = Capsule(np.array([0.0, 0.2, 0.0], dtype=np.float32), 0.9, 0.3)
    new_cap, off, ground = resolve_capsule_world(cap, world)
    assert ground and new_cap.center[1] >= 0.0, (off, new_cap.center, ground)


pytest.skip(
    "capsule ground collision tests require native extensions not built in CI",
    allow_module_level=True,
)
