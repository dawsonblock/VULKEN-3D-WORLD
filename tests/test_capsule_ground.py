import numpy as np

from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class FlatWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0 else 0


def test_capsule_ground_collision() -> None:
    world = FlatWorld()
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    offset, grounded = resolve_capsule_world(cap, world)
    assert grounded
    assert np.isclose(cap.center[1], 1.2, atol=1e-4)
    np.testing.assert_allclose(offset, np.array([0.0, 1.0, 0.0], dtype=np.float32), atol=1e-4)
