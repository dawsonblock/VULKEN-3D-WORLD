import numpy as np
from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class DummyWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:  # pragma: no cover - trivial
        return 1 if y < 0 else 0


def test_capsule_ground_collision() -> None:
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    off, ground = resolve_capsule_world(cap, DummyWorld())
    assert ground
    assert cap.center[1] >= 0.0
    assert off[1] > 0.0
