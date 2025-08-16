import ctypes
import subprocess
from pathlib import Path

import pytest

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

lib.capsule_box_penetration.argtypes = [ctypes.POINTER(Capsule), Vec3, Vec3, ctypes.POINTER(Vec3), ctypes.POINTER(ctypes.c_float)]
lib.capsule_box_penetration.restype = ctypes.c_int

def test_capsule_box_penetration():
    cap = Capsule(Vec3(0.0, 1.2, 0.0), 0.9, 0.3)
    mn = Vec3(-0.5, -0.5, -0.5)
    mx = Vec3(0.5, 0.5, 0.5)
    n = Vec3()
    pen = ctypes.c_float()
    hit = lib.capsule_box_penetration(ctypes.byref(cap), mn, mx, ctypes.byref(n), ctypes.byref(pen))
    assert hit == 1
    assert pen.value > 0
    assert abs(n.y) <= 1.0
