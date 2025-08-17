import numpy as np

from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class DummyWorld:
    """World with solid blocks for ``y < 0`` only."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0 else 0


def test_capsule_ground_collision() -> None:
    cap = Capsule(center=np.array([0.0, 0.2, 0.0], dtype=np.float32), half_height=0.9, radius=0.3)
    off, ground = resolve_capsule_world(cap, DummyWorld())
    assert ground
    assert off[1] > 0
