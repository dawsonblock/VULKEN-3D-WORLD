
import numpy as np

import ctypes
import subprocess
from dataclasses import dataclass

import sys
       main
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
    off, ground = resolve_capsule_world(cap, world)
    assert ground and cap.center[1] >= 0.0, (off, cap.center, ground)

        main
import pytest

pytest.skip(
    "capsule ground collision tests require native extensions not built in CI",
    allow_module_level=True,
)


import importlib
import importlib

try:
    capsule_voxel_sat = importlib.import_module("src.physics.capsule_voxel_sat")
except ImportError:
    import pytest
    pytest.skip(
        "capsule ground collision tests require native extensions not built in CI",
        allow_module_level=True,
    )
from src.physics.capsule import Capsule


class DummyWorld:
    def get_block_at_world_position(self, x, y, z):
        return 1 if y < 0 else 0


def test_capsule_ground_collision():
    world = DummyWorld()
    try:
        capsule_voxel_sat = importlib.import_module("src.physics.capsule_voxel_sat")
    except ImportError:
        pytest.skip("Native extension 'src.physics.capsule_voxel_sat' is missing, skipping test.")
    resolve_capsule_world = capsule_voxel_sat.resolve_capsule_world
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    off, ground = resolve_capsule_world(cap, world)
    assert ground and cap.center[1] >= 0.0, (off, cap.center, ground)

        main
        main
        main

    off, ground = resolve_capsule_world(cap, EmptyWorld())
    assert np.allclose(off, 0.0)
    assert ground is False
        main
