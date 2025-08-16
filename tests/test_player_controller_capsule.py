
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
    raise FileNotFoundError(
        f"Required shared library {LIB_PATH} not found. Please build it before running tests."
    )

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

import numpy as np


        main
from src.physics.player_controller_capsule import (
    PlayerControllerCapsule,
    SPRINT_SPEED_MULTIPLIER,
    get_horizontal_speed,
)



from src.physics import (
    SPRINT_SPEED_MULTIPLIER,
    get_horizontal_speed,
)
from src.physics.player_controller_capsule import PlayerControllerCapsule
        main

SPRINT_SPEED_MULTIPLIER = 1.6


def get_horizontal_speed(player):
    return float(np.linalg.norm(player.vel[[0, 2]]))
        main


class FlatWorld:
    def get_block_at_world_position(self, x, y, z):
        return 1 if y < 0 else 0


class StepWorld:
    def get_block_at_world_position(self, x, y, z):
        if y < 0:
            return 1
        if 0 <= y < 1 and 1 <= x < 2:
            return 1
        return 0


def test_jump_and_land():
    world = FlatWorld()
    player = PlayerControllerCapsule(world, np.array([0.0, 1.2, 0.0], dtype=np.float32))
    forward = np.array([0.0, 0.0, 1.0], dtype=np.float32)
    right = np.array([1.0, 0.0, 0.0], dtype=np.float32)

    player.update(0.1, forward, right)
    assert player.on_ground

    player.set_input({"jump": 1})
    player.update(0.1, forward, right)
    assert player.vel[1] > 0
    assert not player.on_ground

    player.set_input({"jump": 0})
    for _ in range(20):
        player.update(0.1, forward, right)
        if player.on_ground:
            break

    assert player.on_ground
    assert abs(player.pos[1] - 1.2) < 1e-2
    assert player.vel[1] == 0.0


def test_step_climb():
    world = StepWorld()
    player = PlayerControllerCapsule(world, np.array([0.0, 1.2, 0.0], dtype=np.float32))
    forward = np.array([1.0, 0.0, 0.0], dtype=np.float32)
    right = np.array([0.0, 0.0, 1.0], dtype=np.float32)

    player.update(0.1, forward, right)
    player.set_input({"f": 1})
    player.update(0.1, forward, right)

    assert player.pos[1] > 1.2
    assert player.on_ground


def test_sprint_speed_limit():
    world = FlatWorld()
    player = PlayerControllerCapsule(world, np.array([0.0, 1.2, 0.0], dtype=np.float32))
    forward = np.array([1.0, 0.0, 0.0], dtype=np.float32)
    right = np.array([0.0, 0.0, 1.0], dtype=np.float32)

    player.update(0.1, forward, right)
    player.set_input({"f": 1})
    for _ in range(20):
        player.update(0.1, forward, right)

    speed = get_horizontal_speed(player)
    assert speed <= player.max_speed + 1e-3




    speed = get_horizontal_speed(player)
    assert speed <= player.max_speed + 1e-3


    player.set_input({"sprint": 1})
    for _ in range(20):
        player.update(0.1, forward, right)
        main
    speed = get_horizontal_speed(player)
    assert speed <= player.max_speed + 1e-3

        main
        main
    player.set_input({"f": 1, "sprint": 1})
    for _ in range(20):
        player.update(0.1, forward, right)
    sprint_speed = get_horizontal_speed(player)
    assert sprint_speed <= player.max_speed * SPRINT_SPEED_MULTIPLIER + 1e-3
    assert sprint_speed > speed
        main
