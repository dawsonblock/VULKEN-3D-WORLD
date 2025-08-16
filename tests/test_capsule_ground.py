import ctypes
import subprocess
import pytest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LIB_PATH = Path(__file__).with_name("physics_cpp.so")

if not LIB_PATH.exists():
    src = ROOT / "src/physics_cpp/physics_c_api.cpp"
    subprocess.check_call([
        "g++", "-std=c++17", "-shared", "-fPIC", str(src),
        "-I" + str(ROOT / "src/physics_cpp"), "-o", str(LIB_PATH)
    ])

lib = ctypes.CDLL(str(LIB_PATH))

class Vec3(ctypes.Structure):
    _fields_ = [("x", ctypes.c_float), ("y", ctypes.c_float), ("z", ctypes.c_float)]

class Capsule(ctypes.Structure):
    _fields_ = [("center", Vec3), ("half_height", ctypes.c_float), ("radius", ctypes.c_float)]

lib.resolve_capsule_ground.argtypes = [ctypes.POINTER(Capsule), ctypes.POINTER(Vec3)]
lib.resolve_capsule_ground.restype = ctypes.c_int


def test_capsule_ground_collision():
    cap = Capsule(Vec3(0.0, 0.2, 0.0), 0.9, 0.3)
    off = Vec3()
    grounded = lib.resolve_capsule_ground(ctypes.byref(cap), ctypes.byref(off))
    assert grounded == 1
    assert abs(cap.center.y - 1.2) < 1e-4
    assert off.y == pytest.approx(1.0, abs=1e-4)




        main
        main
        main
        main
