import ctypes
from dataclasses import dataclass

import numpy as np
import pytest


@dataclass
class CapsulePy:
    center: np.ndarray
    half_height: float
    radius: float


class Vec3(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
    ]


class Capsule(ctypes.Structure):
    _fields_ = [
        ("center", Vec3),
        ("half_height", ctypes.c_float),
        ("radius", ctypes.c_float),
    ]


def resolve_capsule_world(cap: CapsulePy, world) -> tuple[CapsulePy, np.ndarray, bool]:
    """Resolve a capsule against the ground in a dummy world.

    The capsule is pushed upward if its bottom penetrates the ground at ``y=0``.
    Returns a new capsule with the adjusted center, the applied offset vector,
    and a flag indicating whether ground contact occurred.
    """

    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    ground = False
    new_center = cap.center.copy()

    if world.get_block_at_world_position(cap.center[0], bottom - 1e-4, cap.center[2]):
        delta = -bottom
        new_center[1] += delta
        off[1] = delta
        ground = True

    new_cap = CapsulePy(new_center, cap.half_height, cap.radius)
    return new_cap, off, ground


class DummyWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0.0 else 0


def test_capsule_ground_collision() -> None:
    world = DummyWorld()
    cap = CapsulePy(np.array([0.0, 0.2, 0.0], dtype=np.float32), 0.9, 0.3)
    new_cap, off, ground = resolve_capsule_world(cap, world)

    assert ground is True
    assert new_cap.center[1] == pytest.approx(1.2, abs=1e-4)
    np.testing.assert_allclose(off, np.array([0.0, 1.0, 0.0], dtype=np.float32))
