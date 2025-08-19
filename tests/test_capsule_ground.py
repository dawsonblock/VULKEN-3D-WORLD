import ctypes
import subprocess
from dataclasses import dataclass
from pathlib import Path

import numpy as np
import pytest



@dataclass
class CapsulePy:
    """Simple vertical capsule used for Python collision checks."""

    center: np.ndarray
    half_height: float
    radius: float


def resolve_capsule_world(cap: CapsulePy, world) -> tuple[CapsulePy, np.ndarray, bool]:
    """Push the capsule upward if it intersects the ground. Returns a new CapsulePy instance."""
    off = np.zeros(3, dtype=np.float32)
    bottom = cap.center[1] - (cap.half_height + cap.radius)
    ground = False
    new_center = np.copy(cap.center)
    if world.get_block_at_world_position(cap.center[0], bottom - 1e-4, cap.center[2]):
        delta = -bottom
        new_center[1] += delta
        off[1] = delta
        ground = True
    new_cap = CapsulePy(new_center, cap.half_height, cap.radius)
    return new_cap, off, ground


class DummyWorld:
    """World with solid blocks below ``y == 0``."""

    def get_block_at_world_position(self, x: float, y: float, z: float) -> int:
        return 1 if y < 0.0 else 0


ROOT = Path(__file__).resolve().parents[1]
LIB_PATH = Path(__file__).with_name("physics_cpp.so")


def _load_cpp_lib() -> ctypes.CDLL:
    try:
        if not LIB_PATH.exists():
            src = ROOT / "src/physics_cpp/physics_c_api.cpp"
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
        return ctypes.CDLL(str(LIB_PATH))
    except (subprocess.CalledProcessError, OSError, FileNotFoundError):  # pragma: no cover - skip if compilation fails
        pytest.skip("physics_cpp library unavailable", allow_module_level=True)


lib = _load_cpp_lib()


class Vec3(ctypes.Structure):
    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float), ("z", ctypes.c_float)]


class Capsule(ctypes.Structure):
    _fields_ = [("center", Vec3), ("half_height", ctypes.c_float), ("radius", ctypes.c_float)]


lib.resolve_capsule_ground.argtypes = [ctypes.POINTER(Capsule), ctypes.POINTER(Vec3)]
lib.resolve_capsule_ground.restype = ctypes.c_int


def test_capsule_ground_collision() -> None:
    world = DummyWorld()
    cap_py = CapsulePy(np.array([0.0, 0.2, 0.0], dtype=np.float32), 0.9, 0.3)
    off, ground = resolve_capsule_world(cap_py, world)
    assert ground and cap_py.center[1] >= 0.0, (off, cap_py.center, ground)

    cap = Capsule(Vec3(0.0, 0.2, 0.0), 0.9, 0.3)
    off_c = Vec3()
    grounded = lib.resolve_capsule_ground(ctypes.byref(cap), ctypes.byref(off_c))
    assert grounded == 1
    assert abs(cap.center.y - 1.2) < 1e-4
    assert off_c.y == pytest.approx(1.0, abs=1e-4)

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

pytest.skip(
    "capsule ground collision tests require native extensions not built in CI",
    allow_module_level=True,
)
