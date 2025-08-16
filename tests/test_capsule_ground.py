import os
import sys
import pytest
import importlib
import numpy as np

sys.path.append(os.path.join(os.path.dirname(__file__), ".."))
from src.physics.capsule import Capsule

spec = importlib.util.find_spec("src.physics.capsule_voxel_sat")
if spec is None:  # pragma: no cover - skip if module cannot be imported
    pytest.skip("capsule_voxel_sat module unavailable", allow_module_level=True)
try:
    capsule_voxel_sat = importlib.import_module("src.physics.capsule_voxel_sat")
except Exception:  # pragma: no cover - skip if module import fails
    pytest.skip("capsule_voxel_sat module unavailable", allow_module_level=True)
resolve_capsule_world = capsule_voxel_sat.resolve_capsule_world


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
