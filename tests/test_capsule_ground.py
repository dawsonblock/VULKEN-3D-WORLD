import sys
from pathlib import Path

import numpy as np

sys.path.append(str(Path(__file__).resolve().parents[1]))

from src.physics.capsule import Capsule
from src.physics.capsule_voxel_sat import resolve_capsule_world


class EmptyWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 0


def test_capsule_in_empty_world_has_no_offset() -> None:
    cap = Capsule(
        center=np.array([0.0, 1.0, 0.0], dtype=np.float32),
        half_height=0.5,
        radius=0.25,
    )
    off, ground = resolve_capsule_world(cap, EmptyWorld())
    assert np.allclose(off, 0.0)
    assert ground is False
