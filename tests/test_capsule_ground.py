
import ctypes
import subprocess
from pathlib import Path

import numpy as np
import pytest
from dataclasses import dataclass
from numpy.typing import NDArray


@dataclass
class Capsule:
    center: NDArray[np.float32]
    half_height: float
    radius: float


ROOT = Path(__file__).resolve().parents[1]
LIB_PATH = Path(__file__).with_name("physics_cpp.so")

lib = None
if not LIB_PATH.exists():
    src = ROOT / "src/physics_cpp/physics_c_api.cpp"
    try:
        subprocess.check_call(
            [
                "g++",
                "-std=c++17",
                "-shared",
                "-fPIC",
                str(src),
                "-I" + str(ROOT / "src/physics_cpp"),
                "-o",
                str(LIB_PATH),
            ]
        )
        lib = ctypes.CDLL(str(LIB_PATH))
    except subprocess.CalledProcessError:  # pragma: no cover
        lib = None
else:
    lib = ctypes.CDLL(str(LIB_PATH))


class Vec3(ctypes.Structure):
    _fields_ = [
        ("x", ctypes.c_float),
        ("y", ctypes.c_float),
        ("z", ctypes.c_float),
    ]


class CCapsule(ctypes.Structure):
    _fields_ = [
        ("center", Vec3),
        ("half_height", ctypes.c_float),
        ("radius", ctypes.c_float),
    ]


if lib is not None:
    lib.resolve_capsule_ground.argtypes = [
        ctypes.POINTER(CCapsule),
        ctypes.POINTER(Vec3),
    ]
    lib.resolve_capsule_ground.restype = ctypes.c_int


class DummyWorld:
    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0 else 0


def resolve_capsule_world(
    cap: Capsule, world: DummyWorld
) -> tuple[np.ndarray, bool]:
    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    if world.get_block_at_world_position(0.0, bottom, 0.0):
        delta = -bottom
        cap.center[1] += delta
        off[1] = delta
        return off, True
    return off, False


def test_capsule_ground_collision():
    world = DummyWorld()
    cap = Capsule(
        center=np.array([0.0, 0.2, 0.0], dtype=np.float32),
        half_height=0.9,
        radius=0.3,
    )
    off, ground = resolve_capsule_world(cap, world)
    assert ground and cap.center[1] >= 0.0, (off, cap.center, ground)

    if lib is not None:
        cap_c = CCapsule(Vec3(0.0, 0.2, 0.0), 0.9, 0.3)
        off_c = Vec3()
        grounded = lib.resolve_capsule_ground(
            ctypes.byref(cap_c), ctypes.byref(off_c)
        )
        assert grounded == 1
        assert abs(cap_c.center.y - 1.2) < 1e-4
        assert off_c.y == pytest.approx(1.0, abs=1e-4)
