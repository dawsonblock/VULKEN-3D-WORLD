import numpy as np

from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class GroundWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0.0 else 0


def test_capsule_ground_resolution() -> None:
    cap = Capsule(np.array([0.0, 0.1, 0.0], dtype=np.float32), 0.4, 0.2)
    offset, grounded = resolve_capsule_world(cap, GroundWorld())
    assert grounded
    assert np.allclose(offset, np.array([0.0, 0.5, 0.0], dtype=np.float32))
