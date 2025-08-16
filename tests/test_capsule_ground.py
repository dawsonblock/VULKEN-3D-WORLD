import os
import sys
import numpy as np

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from src.physics.capsule import Capsule


def resolve_capsule_world(cap, world):
    """Minimal ground collision resolution for testing."""
    bottom = cap.center[1] - cap.half_height - cap.radius
    offset = 0.0
    if bottom < 0:
        offset = -bottom
        cap.center[1] += offset
    return np.array([0.0, offset, 0.0], dtype=np.float32), offset > 0


class DummyWorld:
    def get_block_at_world_position(self, x, y, z):
        return 1 if int(y) < 0 else 0  # ground at y=0


def test_capsule_ground_collision():
    world = DummyWorld()
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    off, ground = resolve_capsule_world(cap, world)
    assert ground and cap.center[1] >= 0.0, (off, cap.center, ground)
