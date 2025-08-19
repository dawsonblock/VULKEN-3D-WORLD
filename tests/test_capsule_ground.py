import numpy as np

from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class DummyWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        # Solid ground at y < 0
        return 1 if y < 0.0 else 0


def test_capsule_ground_collision() -> None:
    """A capsule intersecting the ground is pushed upward."""
    world = DummyWorld()
    cap = Capsule(np.array([0.0, 0.2, 0.0], dtype=np.float32), 0.9, 0.3)
    off, ground = resolve_capsule_world(cap, world)
    assert ground
    assert cap.center[1] >= 0.0
    np.testing.assert_allclose(off, np.array([0.0, 1.0, 0.0], dtype=np.float32))
