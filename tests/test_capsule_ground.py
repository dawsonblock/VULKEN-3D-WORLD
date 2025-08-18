import numpy as np
import pytest

pytest.skip(
    "capsule ground collision tests require native extensions not built in CI",
    allow_module_level=True,
)

import importlib

from src.physics.capsule import Capsule


class DummyWorld:
    def get_block_at_world_position(self, x, y, z):
        return 1 if y < 0 else 0


def test_capsule_ground_collision():
    world = DummyWorld()
    capsule_voxel_sat = importlib.import_module("src.physics.capsule_voxel_sat")
    resolve_capsule_world = capsule_voxel_sat.resolve_capsule_world
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    off, ground = resolve_capsule_world(cap, world)
    assert ground and cap.center[1] >= 0.0, (off, cap.center, ground)
